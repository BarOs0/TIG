/**
 * @file TIG_srv.c
 * @brief TIG server implementation - handles client requests and repository storage
 */

#include "TIG_srv.h"
#include "daemon.h"
#include "recv_directory.h"
#include "send_directory.h"
#include "send_file.h"
#include "get_time.h"
#include "copy_directory.h"
#include "mcast_respond.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <limits.h>
#include <sys/wait.h>
#include <pthread.h>

int lock_file_write(int fd){
    struct flock fl = {
        .l_type = F_WRLCK, ///< Exclusive lock - THIS process can write, others blocked
        .l_whence = SEEK_SET, ///< Reference: start of the file
        .l_start = 0, ///< Start of lock: byte 0
        .l_len = 0 ///< Lock range: whole file
    };

    while (fcntl(fd, F_SETLKW, &fl) == -1) { ///< F_SETLKW blocks until lock acquired (blocking); loop handles EINTR (signal)
        if (errno != EINTR)
            return -1;
    }
    return 0;
}

int lock_file_read(int fd){
    struct flock fl = {
        .l_type = F_RDLCK, ///< Shared lock - THIS process can read, others can read too, writing blocked
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0
    };
    while (fcntl(fd, F_SETLKW, &fl) == -1) {
        if (errno != EINTR)
            return -1;
    }
    return 0;
}

int unlock_file(int fd){
    struct flock fl = {
        .l_type = F_UNLCK, ///< Unlock a file (read / write)
        .l_whence = SEEK_SET, ///< Reference: start of the file
        .l_start = 0, ///< Start of unlock: byte 0
        .l_len = 0 ///< Unlock range: whole file
    };
    return fcntl(fd, F_SETLK, &fl); ///< Immediately unlock, F_SETLK: non blocking unlock 
}

void handle_client(int connfd, struct sockaddr_in6 *cliaddr) {
    char cmd; ///< Command form client 
    char commit_buff[COMMIT_BUFF_SIZE] = {0}; ///< commit message buffer
    char path_buff[PATH_MAX] = {0}; ///< path buff 
    char path_backups[PATH_MAX] = {0}; ///< path buff for backups directory
    char name_buff[NAME_BUFF_SIZE] = {0}; ///< repo name buffer
    char lock_path[PATH_MAX] = {0}; ///< file to "lock" directory
    char time_str[TIME_BUFF_SIZE] = {0}; ///< time stamp buffer for logging
    FILE *f; ///< file descriptor

    struct sockaddr_in6 peeraddr; ///< peer addr for logging
    socklen_t peeraddr_len = sizeof(peeraddr);
    char peeraddr_str[INET6_ADDRSTRLEN] = {0};
    bzero(&peeraddr, sizeof(peeraddr));
    if(getpeername(connfd, (struct sockaddr*) &peeraddr, &peeraddr_len) < 0){ ///< Get peer address (useful for logging and data storage)
        syslog(LOG_ERR, " TIG_srv.c getpeername() error: %s", strerror(errno));
        return;
    }

    if(inet_ntop(AF_INET6, &peeraddr.sin6_addr, peeraddr_str, INET6_ADDRSTRLEN) < 0){ ///< Convert peer address to presentation form
        syslog(LOG_ERR, " TIG_srv.c inet_ntop() error: %s", strerror(errno));
        return;
    }
    
    get_time(time_str, TIME_BUFF_SIZE); ///< Get current time (useful for logging and data storage)

    if(read(connfd, &cmd, 1) < 0){ ///< Read commands from peer
        syslog(LOG_ERR, " TIG_srv.c read() cmd error: %s", strerror(errno));
        return;
    }

    switch(cmd){

        case 'U': ///< 'U' - pull
            if(read(connfd, name_buff, NAME_BUFF_SIZE) < 0){ ///< Read the name of the pulled repository
                syslog(LOG_ERR, " TIG_srv.c read() name pull error: %s", strerror(errno));
                return;
            }

            /** LOCK REPOSITORY FOR READING */

            strcpy(lock_path, DATA_PATH); ///< Creating lock file path
            strcat(lock_path, "/repos/.lock_");
            strcat(lock_path, name_buff);
            
            int lockfd_pull = open(lock_path, O_CREAT | O_RDONLY, 0644); ///< Check if lock file is created
            if(lockfd_pull < 0){
                syslog(LOG_ERR, " cannot create lock file for pull: %s", strerror(errno));
                return;
            }

            if(lock_file_read(lockfd_pull) < 0){ ///< Lock file (whole repo)
                syslog(LOG_ERR, " lock_file_read() error: %s", strerror(errno));
                close(lockfd_pull);
                return;
            }

            syslog(LOG_INFO, " HOST: %s, %s: %s\n", peeraddr_str, "pulled", name_buff); ///< LOGGING FOR ADMINISTRATOR (big brother ;))

            strcpy(path_buff, DATA_PATH); ///< Prepare repo path
            strcat(path_buff, "/repos/");
            strcat(path_buff, name_buff);
            send_directory(connfd, path_buff); ///< Send stored directory to peer

            unlock_file(lockfd_pull); ///< Unlock file 
            close(lockfd_pull); ///< Close file descriptor
            break;

        case 'R': ///< 'R' - repos
            syslog(LOG_INFO, " HOST: %s, %s\n", peeraddr_str, "listed repos"); ///< LOGGING FOR ADMINISTRATOR 
            strcpy(path_buff, DATA_PATH); ///< Prepare repo path 
            strcat(path_buff, "/list");
            
            f = fopen(path_buff, "r");
            if(f == NULL){
                syslog(LOG_WARNING, " /srv/data/list file does not exist, reqesting HOST: %s\n", peeraddr_str);
                break;
            }

            if(lock_file_read(fileno(f)) < 0){
                syslog(LOG_ERR, " lock_file_read() error: %s", strerror(errno));
                fclose(f);
                return;
            }

            send_file(connfd, path_buff); ///< Send repository list to peer
            
            unlock_file(fileno(f));
            fclose(f);
            break;
        
        case 'P': ///< 'P' - push 

            if(read(connfd, name_buff, NAME_BUFF_SIZE) < 0){ ///< Read the name of the pushed repository
                syslog(LOG_ERR, " TIG_srv.c read() name push error: %s", strerror(errno));
                return;
            }

            /** LOCK REPOSITORY DIRECTORY FOR WRITING */

            strcpy(lock_path, DATA_PATH); ///< Creating lock file path
            strcat(lock_path, "/repos/.lock_");
            strcat(lock_path, name_buff);
            
            int lockfd_push = open(lock_path, O_CREAT | O_WRONLY, 0644); ///< Check if lock file is created
            if(lockfd_push < 0){
                syslog(LOG_ERR, " cannot create lock file: %s", strerror(errno));
                return;
            }

            if(lock_file_write(lockfd_push) < 0){  ///< LOCK entire /srv/data/repo for writing
                syslog(LOG_ERR, " lock_file() repo error: %s", strerror(errno));
                close(lockfd_push);
                unlink(lock_path);
                return;
            }

            strcpy(path_buff, DATA_PATH); ///< Prepare path for data/repos storage directory
            strcat(path_buff, "/repos/");
            strcat(path_buff, name_buff);

            strcpy(path_backups, DATA_PATH); ///< Prepare path for data/backups storage directory
            strcat(path_backups, "/backups/");
            strcat(path_backups, name_buff);

            copy_directory(path_buff, path_backups); ///< Copy present repository from data/repos to data/backups

            syslog(LOG_INFO, " HOST: %s, %s: %s\n", peeraddr_str, "pushed", name_buff); ///< LOGGING FOR ADMINISTRATOR 

            strcpy(path_buff, DATA_PATH); ///< Prepare path 
            strcat(path_buff, "/repos");
            recv_directory(connfd, path_buff); ///< Receive whole directory to data storage

            /** CREATING AND UPDATING REPOSITORY LIST */

            struct stat st = {0};
            strcpy(path_buff, DATA_PATH); ///< Prepare list path
            strcat(path_buff, "/list");
            if(stat(path_buff, &st) == -1){
                FILE* touch = fopen(path_buff, "a"); ///< Create list file
                if(touch == NULL){
                    syslog(LOG_ERR, " cannot create /srv/data/list: %s", strerror(errno));
                    unlock_file(lockfd_push);
                    close(lockfd_push);
                    unlink(lock_path);
                    return;
                }
                fclose(touch);
            }

            f = fopen(path_buff, "r+"); ///< Open file for read and write
            if(f == NULL){
                syslog(LOG_ERR, " counting repos in /srv/data/list error: %s", strerror(errno));
                unlock_file(lockfd_push);
                close(lockfd_push);
                unlink(lock_path);
                return;
            }

            if(lock_file_write(fileno(f)) < 0){  ///< Lock file /srv/data/list
                syslog(LOG_ERR, " lock_file() error: %s", strerror(errno));
                fclose(f);
                unlock_file(lockfd_push);
                close(lockfd_push);
                unlink(lock_path);
                return;
            }

            int i = 0;
            int repo_name_exist = 0; ///< Flag (if repo name is already listed)
            char line[256];
            rewind(f); ///< Point list file to position 0 (start of the file)
            while(fgets(line, sizeof(line), f)){
                if(line[0] == '\n' || line[0] == '\0') continue; ///< Skip '\n' and '\0'
                i++; ///< Increment lines
                char *repo_name_space = strchr(line, ' '); ///< Format: 1.' '<repo_name>
                if(repo_name_space){
                    char* name = repo_name_space + 1; ///< One char after space (pure repo name)
                    char* newline = strchr(name, '\n'); 
                    if(newline){ ///< Delete '\n'
                        *newline = '\0';
                    }
                    if(strcmp(name, name_buff) == 0){ ///< If repo name is already listed -> set repo_name_exist flag
                        repo_name_exist = 1;
                        break;
                    }
                }
            }
            if(repo_name_exist == 0){
                fseek(f, 0, SEEK_END); ///< Go to end of file
                fprintf(f, "%d. %s\n", i + 1, name_buff); ///< Update list file
                fflush(f);
            }

            if(unlock_file(fileno(f)) < 0){ ///< Unlock list file (via descriptor)
                syslog(LOG_WARNING, " unlock_file() list failed: %s", strerror(errno));
            }
            fclose(f); 

            if(unlock_file(lockfd_push) < 0){ ///< Unlock locked file (repos locking)
                syslog(LOG_WARNING, " unlock_file() repo failed: %s", strerror(errno));
            } 
            close(lockfd_push); ///< Close lock file descriptor
            unlink(lock_path); ///< Delete locked file (repos locking)
            break;

        case 'C': ///< 'C' - commit
            if(read(connfd, name_buff, NAME_BUFF_SIZE) < 0){ ///< Read the name of the committed repository
                syslog(LOG_ERR, " TIG_srv.c read() name commit error: %s", strerror(errno));
                return;
            }

            syslog(LOG_INFO, " HOST: %s, %s: %s\n", peeraddr_str, "commited", name_buff); ///< LOGGING FOR ADMINISTRATOR 

            if(read(connfd, commit_buff, COMMIT_BUFF_SIZE) < 0){ ///< Read the message of the commit
                syslog(LOG_ERR, " TIG_srv.c read() commit commit error: %s", strerror(errno));
                return;
            }

            strcpy(path_buff, DATA_PATH); ///< Prepare path
            strcat(path_buff, "/commits/");
            strcat(path_buff, name_buff);

            f = fopen(path_buff, "a"); ///< Open or create commit data storage 
            if(f == NULL){
                syslog(LOG_ERR, " open commit file error: %s", strerror(errno));
                return;
            }

            /** LOCK COMMIT FILE*/

            if(lock_file_write(fileno(f)) < 0){ /// Lock for writing commit (repo) file
                syslog(LOG_ERR, " lock_file() error: %s", strerror(errno));
                fclose(f);
                return;
            }

            fprintf(f, "%s: HOST: %s, MESSAGE: %s\n", time_str, peeraddr_str, commit_buff); ///< Update commits file with time and peer address
            fflush(f); ///< Flush data

            if(unlock_file(fileno(f)) < 0){ ///< Unlock commit (repo) file
                syslog(LOG_WARNING, " unlock_file() commits failed: %s", strerror(errno));
            } 
            fclose(f);
            break;

        case 'Q': ///< 'Q' - read commits
            if(read(connfd, name_buff, NAME_BUFF_SIZE) < 0){ ///< Read the name of the committed repository
                syslog(LOG_ERR, " TIG_srv.c read() name commit error: %s", strerror(errno));
                return;
            }

            syslog(LOG_INFO, " HOST: %s, %s %s\n", peeraddr_str, "read commits related to:", name_buff); ///< LOGGING FOR ADMINISTRATOR

            strcpy(path_buff, DATA_PATH); ///< Prepare path 
            strcat(path_buff, "/commits/");
            strcat(path_buff, name_buff);

            f = fopen(path_buff, "r");
            if(f == NULL){
                syslog(LOG_WARNING, " commits file does not exist for: %s", name_buff);
                break;
            }

            if(lock_file_read(fileno(f)) < 0){ ///< Lock file
                syslog(LOG_ERR, " lock_file_read() error: %s", strerror(errno));
                fclose(f);
                return;
            }

            send_file(connfd, path_buff); ///< Send repository commits to peer
            
            unlock_file(fileno(f)); ///< Unlock file
            fclose(f);
            break;
    }
}

void sigchld_handler(int signo){ ///< ZOMBIE TERMINATION
    (void)signo;
    while (waitpid(-1, NULL, WNOHANG) > 0); ///< The walking dead 
}

void run(void) {
    int listenfd, connfd;
    struct sockaddr_in6 servaddr, cliaddr;
    socklen_t size;

    bzero(&servaddr, sizeof(servaddr));
    bzero(&cliaddr, sizeof(cliaddr));

    struct sigaction sa; ///< SIGCHLD interception
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; ///< Blocking functions will continue 
    sigaction(SIGCHLD, &sa, NULL);

    if((listenfd = socket(AF_INET6, SOCK_STREAM, 0)) < 0){ ///< Creating listening socket
        syslog(LOG_ERR, " listening socket() error: %s", strerror(errno));
        exit(1);
    }

    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_addr = in6addr_any;
    servaddr.sin6_port = htons(LISTEN_PORT_FTP);

    if(bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0){ ///< Binding listening socket, port 2025, ADDR_ANY
        syslog(LOG_ERR, " listening socket bind() error: %s", strerror(errno));
        exit(1);
    }

    if(listen(listenfd, LISTENQ) < 0){ ///< Listen 
        syslog(LOG_ERR, " listen() error: %s", strerror(errno));
        exit(1);
    }

    char time_str[TIME_BUFF_SIZE] = {0};
    get_time(time_str, TIME_BUFF_SIZE);
    syslog(LOG_INFO, "%s\n", " System ready, waiting for clients..."); ///< LOGGING FOR ADMINISTRATOR 

    while(1){

        /** CONCURRENCY USING FORK() */

        size = sizeof(cliaddr);
        if((connfd = accept(listenfd, (struct sockaddr*) &cliaddr, &size)) < 0){ ///< Accept client calls
            syslog(LOG_ERR, " accept() error: %s", strerror(errno));
            continue;
        }
        
        pid_t pid = fork();
        if (pid < 0){
            syslog(LOG_ERR, " fork() error: %s", strerror(errno));
            close(connfd);
            continue;
        } 
        else if (pid == 0){  ///< If child
            close(listenfd); 
            handle_client(connfd, &cliaddr);
            close(connfd);
            exit(0);
        }
        else{ ///< If parent
            close(connfd);
        }
    }
}

/**
 * @brief Main function for TIG server - initializes daemon and starts server
 * @param argc Argument count
 * @param argv Argument vector
 * @return 0 on success
 */
int main(int argc, char** argv){
    daemon_init("TIG_srv", LOG_DAEMON, getuid());
    openlog("TIG_srv", LOG_PID, LOG_DAEMON);
    pthread_t mcast_respond_thread; ///< UDP mcast_discovery thread
    pthread_create(&mcast_respond_thread, NULL, mcast_respond, NULL);
    run();
    return 0;
}