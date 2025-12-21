#include "TIG_cli.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int connection(const char *server_addr) {
    int n = 0, err = 0, sockfd = 0;
    char buff[BUFF_SIZE];
    struct sockaddr_in6 servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_port = htons(LISTEN_PORT_FTP);

    if((sockfd = socket(AF_INET6, SOCK_STREAM, 0)) < 0){
        fprintf(stderr, "socket() error: %s\n", strerror(errno));
        return -1;
    }

    if((err = inet_pton(AF_INET6, server_addr, &servaddr.sin6_addr)) <= 0){
        if(err == 0){
            fprintf(stderr, "inet_pton error for %s \n", server_addr);
        } else {
            fprintf(stderr, "inet_pton error for %s : %s \n", server_addr, strerror(errno));
        }
        return -1;
    }

    if(connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0){
        fprintf(stderr, "connect() error: %s \n", strerror(errno));
        return -1;
    }

    while((n = read(sockfd, buff, BUFF_SIZE-1)) > 0){
        buff[n] = 0;
        if(fputs(buff, stdout) == EOF){
            fprintf(stderr, "fputs() error: %s\n", strerror(errno));
            return -1;
        }
    }

    if (n < 0)
        fprintf(stderr,"read error : %s\n", strerror(errno));

    fflush(stdout);
    fprintf(stderr, "\nOK\n");
    close(sockfd);

    return 0;
}

int main(int argc, char **argv){
    if(argc != 2){
        fprintf(stderr, "ERROR: usage: ./TIG_cli <Serv_addr> \n");
        return -1;
    }
    return connection(argv[1]);
}