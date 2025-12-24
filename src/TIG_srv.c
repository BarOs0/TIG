#include "TIG_srv.h"
#include "daemon.h"
#include "recv_directory.h"
#include "send_directory.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <time.h>
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

void handle_client(int connfd, struct sockaddr_in6 *cliaddr) {
    char cmd;
    char commit_buff[COMMIT_BUFF_SIZE] = {0};
    char repos_buff[REPOS_BUFF_SIZE] = {0};
    char path_buff[PATH_MAX] = {0};
    char name_buff[NAME_BUFF_SIZE] = {0};

    if(read(connfd, &cmd, 1) < 0){
        syslog(LOG_ERR, "read() cmd error: %s", strerror(errno));
        exit(1);
    }

    switch(cmd){

        case 'U':
            if(read(connfd, name_buff, NAME_BUFF_SIZE) < 0){
                syslog(LOG_ERR, "read() name error: %s", strerror(errno));
                exit(1);
            }
            strcpy(path_buff, REPOS_PATH);
            strcat(path_buff, "/repos/");
            strcat(path_buff, name_buff);
            send_directory(connfd, path_buff);
            break;

        case 'R':
            strcpy(path_buff, REPOS_PATH);
            strcat(path_buff, "/list");
            send_directory(connfd, path_buff);
            break;
        
        case 'P':
            if(read(connfd, name_buff, NAME_BUFF_SIZE) < 0){
                syslog(LOG_ERR, "read() name error: %s", strerror(errno));
                exit(1);
            }
            strcpy(path_buff, REPOS_PATH);
            strcat(path_buff, "/repos/");
            strcat(path_buff, name_buff);
            recv_directory(connfd, path_buff);

            struct stat st = {0};
            strcpy(path_buff, REPOS_PATH);
            strcat(path_buff, "/list");
            if(stat(path_buff, &st) == -1){
                mkdir(path_buff, 0755);
            }

            FILE* f = fopen(path_buff, "a+");
            if(f == NULL){
                syslog(LOG_ERR, "counting repos in /srv/data/list error: %s", strerror(errno));
                exit(1);
            }
            int i = 0;
            char line[256];
            rewind(f);
            while(fgets(line, sizeof(line), f)){
                i++;
            }
            fprintf(f, "%d. %s\n", i + 1, name_buff);
            close(f);
            break;

        case 'C':
            if(read(connfd, name_buff, NAME_BUFF_SIZE) < 0){
                syslog(LOG_ERR, "read() name error: %s", strerror(errno));
                exit(1);
            }

            if(read(connfd, commit_buff, COMMIT_BUFF_SIZE) < 0){
                syslog(LOG_ERR, "read() commit error: %s", strerror(errno));
                exit(1);
            }
            
            strcpy(path_buff, REPOS_PATH);
            strcat(path_buff, "/commits/");
            strcat(path_buff, name_buff);

            time_t now = time(NULL);
            struct tm *tm_info = localtime(&now);
            char time_str[64];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

            FILE *f = fopen(path_buff, "a");
            if(f == NULL){
                syslog(LOG_ERR, "open commit file error: %s", strerror(errno));
                exit(1);
            }
            fprintf(f, "%s: %s\n", time_str, commit_buff);
            fclose(f);
            break;
    }
}

void run(void) {
    int listenfd, connfd;
    struct sockaddr_in6 servaddr, cliaddr;
    socklen_t size;

    bzero(&servaddr, sizeof(servaddr));
    bzero(&cliaddr, sizeof(cliaddr));

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

    while(1){
        size = sizeof(cliaddr);
        if((connfd = accept(listenfd, (struct sockaddr*) &cliaddr, &size)) < 0){
            syslog(LOG_ERR, "socket() error: %s", strerror(errno));
            continue;
        }
        handle_client(connfd, &cliaddr);
        close(connfd);
    }
}

int main(int argc, char** argv){
    daemon_init("TIG_srv", LOG_DAEMON, getuid());
    openlog("TIG_srv", LOG_PID, LOG_DAEMON);
    run();
    return 0;
}