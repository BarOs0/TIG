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

void handle_client(int connfd, struct sockaddr_in6 *cliaddr) {
    char cmd;
    char commit_buff[COMMIT_BUFF_SIZE] = {0};
    char path_buff[PATH_MAX] = {0};
    char path_buff_2[PATH_MAX] = {0};
    char name_buff[NAME_BUFF_SIZE] = {0};
    char time_str[TIME_BUFF_SIZE] = {0};
    FILE *f;

    struct sockaddr_in6 peeraddr;
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

            syslog(LOG_INFO, " HOST: %s, %s: %s\n", peeraddr_str, "pulled", name_buff); ///< LOGGING FOR ADMINISTRATOR (big brother ;))

            strcpy(path_buff, REPOS_PATH); ///< Prepare path
            strcat(path_buff, "/repos/");
            strcat(path_buff, name_buff);
            send_directory(connfd, path_buff); ///< Send stored directory to peer
            break;

        case 'R': ///< 'R' - repos
            syslog(LOG_INFO, " HOST: %s, %s\n", peeraddr_str, "listed repos"); ///< LOGGING FOR ADMINISTRATOR 
            strcpy(path_buff, REPOS_PATH); ///< Prepare path 
            strcat(path_buff, "/list");
            send_file(connfd, path_buff); ///< Send repository list to peer
            break;
        
        case 'P': ///< 'P' - push 

            if(read(connfd, name_buff, NAME_BUFF_SIZE) < 0){ ///< Read the name of the pushed repository
                syslog(LOG_ERR, " TIG_srv.c read() name push error: %s", strerror(errno));
                return;
            }

            strcpy(path_buff, REPOS_PATH); ///< Prepare path for data/repos storage directory
            strcat(path_buff, "/repos/");
            strcat(path_buff, name_buff);

            strcpy(path_buff_2, REPOS_PATH); ///< Prepare path for data/backups storage directory
            strcat(path_buff_2, "/backups/");
            strcat(path_buff_2, name_buff);

            copy_directory(path_buff, path_buff_2); ///< Copy present repository from data/repos to data/backups

            syslog(LOG_INFO, " HOST: %s, %s: %s\n", peeraddr_str, "pushed", name_buff); ///< LOGGING FOR ADMINISTRATOR 

            strcpy(path_buff, REPOS_PATH); ///< Prepare path 
            strcat(path_buff, "/repos");
            recv_directory(connfd, path_buff); ///< Receive whole directory to data storage

            /** CREATING AND UPDATING REPOSITORY LIST */

            struct stat st = {0};
            strcpy(path_buff, REPOS_PATH);
            strcat(path_buff, "/list");
            if(stat(path_buff, &st) == -1){
                FILE* touch = fopen(path_buff, "a"); ///< Create list file
                if(touch == NULL){
                    syslog(LOG_ERR, " cannot create /srv/data/list: %s", strerror(errno));
                    return;
                }
                fclose(touch);
            }

            f = fopen(path_buff, "a+"); ///< Open file and count lines
            if(f == NULL){
                syslog(LOG_ERR, " counting repos in /srv/data/list error: %s", strerror(errno));
                return;
            }
            int i = 0;
            int repo_name_exist = 0; ///< Flag (if repo name is already listed)
            char line[256];
            rewind(f);
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
                fprintf(f, "%d. %s\n", i + 1, name_buff); ///< Update list file
            }
            fclose(f);
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

            strcpy(path_buff, REPOS_PATH); ///< Prepare path
            strcat(path_buff, "/commits/");
            strcat(path_buff, name_buff);

            f = fopen(path_buff, "a"); ///< Open or create commit data storage 
            if(f == NULL){
                syslog(LOG_ERR, " open commit file error: %s", strerror(errno));
                return;
            }
            fprintf(f, "%s: HOST: %s, MESSAGE: %s\n", time_str, peeraddr_str, commit_buff); ///< Update commits file with time and peer address
            fclose(f);
            break;

        case 'Q': ///< 'Q' - read commits
            if(read(connfd, name_buff, NAME_BUFF_SIZE) < 0){ ///< Read the name of the committed repository
                syslog(LOG_ERR, " TIG_srv.c read() name commit error: %s", strerror(errno));
                return;
            }

            syslog(LOG_INFO, " HOST: %s, %s %s\n", peeraddr_str, "read commits related to:", name_buff); ///< LOGGING FOR ADMINISTRATOR

            strcpy(path_buff, REPOS_PATH); ///< Prepare path 
            strcat(path_buff, "/commits/");
            strcat(path_buff, name_buff);
            send_file(connfd, path_buff); ///< Send repository commits to peer
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
    syslog(LOG_INFO, " %s\n", " System ready, waiting for clients..."); ///< LOGGING FOR ADMINISTRATOR 

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

int main(int argc, char** argv){
    daemon_init("TIG_srv", LOG_DAEMON, getuid());
    openlog("TIG_srv", LOG_PID, LOG_DAEMON);
    pthread_t mcast_respond_thread; ///< UDP mcast_discovery thread
    pthread_create(&mcast_respond_thread, NULL, mcast_respond, NULL);
    run();
    return 0;
}