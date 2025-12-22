#include "TIG_srv.h"
#include "daemon.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
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

void handle_client(int connfd, struct sockaddr_in6 *cliaddr) {
    char peer_addr[INET6_ADDRSTRLEN+1];
    char buff[BUFF_SIZE];
    time_t ticks = time(NULL);

    inet_ntop(AF_INET6, (void*)&cliaddr->sin6_addr, peer_addr, sizeof(peer_addr));
    syslog(LOG_INFO, "Connection from: %s", peer_addr);

    snprintf(buff, BUFF_SIZE, "%.24s\r\n", ctime(&ticks));
    if(write(connfd, buff, strlen(buff)) < 0){
        syslog(LOG_ERR, "socket() error: %s", strerror(errno));
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