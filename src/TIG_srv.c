#include        <sys/types.h>   /* basic system data types */
#include        <sys/socket.h>  /* basic socket definitions */
#include        <sys/time.h>    /* timeval{} for select() */
#include        <time.h>                /* timespec{} for pselect() */
#include        <netinet/in.h>  /* sockaddr_in{} and other Internet defns */
#include        <arpa/inet.h>   /* inet(3) functions */
#include        <errno.h>
#include        <fcntl.h>               /* for nonblocking */
#include        <netdb.h>
#include        <signal.h>
#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include        <unistd.h>

#define LISTENQ 2
#define LISTEN_PORT_FTP 21
#define BUFF_SIZE 1024 //debug

int main(int argc, char** argv){

    time_t ticks = 0;//debug

    int listenfd = 0;
    int connfd = 0;

    struct sockaddr_in6 servaddr;
    bzero(&servaddr, sizeof(servaddr));

    struct sockaddr_in6 cliaddr;
    bzero(&cliaddr, sizeof(cliaddr));

    socklen_t size = 0;

    if((listenfd = socket(AF_INET6, SOCK_STREAM, 0)) < 0){
        fprintf(stderr, "socket() error: %s\n", strerror(errno)); //debug
        return -1;
    }

    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_addr = in6addr_any;
    servaddr.sin6_port = htons(LISTEN_PORT_FTP);

    if(bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0){
        fprintf(stderr, "bind() error: %s\n", strerror(errno)); //debug
        return -1;
    }

    if(listen(listenfd, LISTENQ) < 0){
        fprintf(stderr, "listen() error: %s\n", strerror(errno)); //debug
        return -1;
    }

    char peer_addr[INET6_ADDRSTRLEN+1];
    char buff[BUFF_SIZE]; //debuhg

    while(1){

        size = sizeof(cliaddr);

        if((connfd = accept(listenfd, (struct sockaddr*) &cliaddr, &size)) < 0){
            fprintf(stderr, "accept() error: %s\n", strerror(errno)); //debug
            continue;
        }

        inet_ntop(AF_INET6, (struct sockaddr*) &cliaddr.sin6_addr, peer_addr, sizeof(peer_addr));
        printf("Connection from: %s\n", peer_addr);

        ticks = time(NULL); //debug
        snprintf(buff, BUFF_SIZE, "%.24s\r\n", ctime(&ticks));
        if(write(connfd, buff, strlen(buff)) < 0){
            fprintf(stderr, "write() error: %s\n", strerror(errno));
        }

        close(connfd);

    }
}