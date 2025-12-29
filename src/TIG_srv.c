#include "TIG_srv.h"
#include "daemon.h"
#include "recv_directory.h"
#include "send_directory.h"
#include "send_file.h"
#include "get_time.h"
#include "copy_directory.h"
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
    if(getpeername(connfd, (struct sockaddr*) &peeraddr, &peeraddr_len) < 0){
        syslog(LOG_ERR, "TIG_srv.c getpeername() error: %s", strerror(errno));
        return;
    }

    if(inet_ntop(AF_INET6, &peeraddr.sin6_addr, peeraddr_str, INET6_ADDRSTRLEN) < 0){
        syslog(LOG_ERR, "TIG_srv.c inet_ntop() error: %s", strerror(errno));
        return;
    }
    
    get_time(time_str, TIME_BUFF_SIZE);

    if(read(connfd, &cmd, 1) < 0){
        syslog(LOG_ERR, "TIG_srv.c read() cmd error: %s", strerror(errno));
        return;
    }

    switch(cmd){

        case 'U': //pull
            if(read(connfd, name_buff, NAME_BUFF_SIZE) < 0){
                syslog(LOG_ERR, "TIG_srv.c read() name pull error: %s", strerror(errno));
                return;
            }

            syslog(LOG_INFO, "%s: HOST: %s, %s: %s\n", time_str, peeraddr_str, "pulled", name_buff);

            strcpy(path_buff, REPOS_PATH);
            strcat(path_buff, "/repos/");
            strcat(path_buff, name_buff);
            send_directory(connfd, path_buff);
            break;

        case 'R':
            syslog(LOG_INFO, "%s: HOST: %s, %s\n", time_str, peeraddr_str, "listed repos");
            strcpy(path_buff, REPOS_PATH);
            strcat(path_buff, "/list");
            send_file(connfd, path_buff);
            break;
        
        case 'P': //push

            if(read(connfd, name_buff, NAME_BUFF_SIZE) < 0){
                syslog(LOG_ERR, "TIG_srv.c read() name push error: %s", strerror(errno));
                return;
            }

            strcpy(path_buff, REPOS_PATH);
            strcat(path_buff, "/repos/");
            strcat(path_buff, name_buff);

            strcpy(path_buff_2, REPOS_PATH);
            strcat(path_buff_2, "/backups/");
            strcat(path_buff_2, name_buff);

            copy_directory(path_buff, path_buff_2);

            syslog(LOG_INFO, "%s: HOST: %s, %s: %s\n", time_str, peeraddr_str, "pushed", name_buff);

            strcpy(path_buff, REPOS_PATH);
            strcat(path_buff, "/repos");
            recv_directory(connfd, path_buff);

            struct stat st = {0};
            strcpy(path_buff, REPOS_PATH);
            strcat(path_buff, "/list");
            if(stat(path_buff, &st) == -1){
                FILE* touch = fopen(path_buff, "a");
                if(touch == NULL){
                    syslog(LOG_ERR, "cannot create /srv/data/list: %s", strerror(errno));
                    return;
                }
                fclose(touch);
            }

            f = fopen(path_buff, "a+");
            if(f == NULL){
                syslog(LOG_ERR, "counting repos in /srv/data/list error: %s", strerror(errno));
                return;
            }
            int i = 0;
            char line[256];
            rewind(f);
            while(fgets(line, sizeof(line), f)){
                if(line[0] == '\n' || line[0] == '\0') continue;
                i++;
            }
            fprintf(f, "%d. %s\n", i + 1, name_buff);
            fclose(f);
            break;

        case 'C':
            if(read(connfd, name_buff, NAME_BUFF_SIZE) < 0){
                syslog(LOG_ERR, "TIG_srv.c read() name commit error: %s", strerror(errno));
                return;
            }

            syslog(LOG_INFO, "%s: HOST: %s, %s: %s\n", time_str, peeraddr_str, "commited", name_buff);

            if(read(connfd, commit_buff, COMMIT_BUFF_SIZE) < 0){
                syslog(LOG_ERR, "TIG_srv.c read() commit commit error: %s", strerror(errno));
                return;
            }

            strcpy(path_buff, REPOS_PATH);
            strcat(path_buff, "/commits/");
            strcat(path_buff, name_buff);

            f = fopen(path_buff, "a");
            if(f == NULL){
                syslog(LOG_ERR, "open commit file error: %s", strerror(errno));
                return;
            }
            fprintf(f, "%s: HOST: %s, MESSAGE: %s\n", time_str, peeraddr_str, commit_buff);
            fclose(f);
            break;
    }
}

void sigchld_handler(int signo){
    (void)signo;
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void run(void) {
    int listenfd, connfd;
    struct sockaddr_in6 servaddr, cliaddr;
    socklen_t size;

    bzero(&servaddr, sizeof(servaddr));
    bzero(&cliaddr, sizeof(cliaddr));

    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);

    if((listenfd = socket(AF_INET6, SOCK_STREAM, 0)) < 0){
        syslog(LOG_ERR, "socket() error: %s", strerror(errno));
        exit(1);
    }

    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_addr = in6addr_any;
    servaddr.sin6_port = htons(LISTEN_PORT_FTP);

    if(bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0){
        syslog(LOG_ERR, "socket() error: %s", strerror(errno));
        exit(1);
    }

    if(listen(listenfd, LISTENQ) < 0){
        syslog(LOG_ERR, "socket() error: %s", strerror(errno));
        exit(1);
    }

    char time_str[TIME_BUFF_SIZE] = {0};
    get_time(time_str, TIME_BUFF_SIZE);
    syslog(LOG_INFO, "%s: %s\n", time_str, "System ready, waiting for clients...");

    while(1){
        size = sizeof(cliaddr);
        if((connfd = accept(listenfd, (struct sockaddr*) &cliaddr, &size)) < 0){
            syslog(LOG_ERR, "socket() error: %s", strerror(errno));
            continue;
        }
        
        pid_t pid = fork();
        if (pid < 0){
            syslog(LOG_ERR, "fork() error: %s", strerror(errno));
            close(connfd);
            continue;
        } 
        else if (pid == 0){ 
            close(listenfd); 
            handle_client(connfd, &cliaddr);
            close(connfd);
            exit(0);
        } 
        else{ 
            close(connfd);
        }
    }
}

int main(int argc, char** argv){
    daemon_init("TIG_srv", LOG_DAEMON, getuid());
    openlog("TIG_srv", LOG_PID, LOG_DAEMON);
    run();
    return 0;
}