#include "mcast_discover.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <fcntl.h>
#include <net/if.h>

int mcast_discover(void){

    int sockfd, n;
    struct sockaddr_in6 mcastaddr, servaddr;

    bzero(&mcastaddr, sizeof(mcastaddr));
    bzero(&servaddr, sizeof(servaddr));
    socklen_t len = sizeof(servaddr);

    char msg[MSG_SIZE] = {0};
    char srvaddrstr[INET6_ADDRSTRLEN] = {0};

    if((sockfd = socket(AF_INET6, SOCK_DGRAM,0)) < 0){
        fprintf(stderr, "mcast_discover.c socket() error: %s\n", strerror(errno));
        return -1;
    }

    int ifindex;
    if((ifindex = if_nametoindex(MCAST_IF)) == 0){
        fprintf(stderr, "mcast_discover.c interface: %s unreachable\n", MCAST_IF);
        close(sockfd);
        return -1;
    }

    if(setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_IF, &ifindex, sizeof(ifindex)) < 0){
        fprintf(stderr, "mcast_discover.c setsockopt(ifindex) error: %s\n", strerror(errno));
        close(sockfd);
        return -1;
    }

    mcastaddr.sin6_family = AF_INET6;
    mcastaddr.sin6_port = htons(MCAST_PORT);

    if(inet_pton(AF_INET6, MCAST_ADDR, &mcastaddr.sin6_addr) < 0){
        fprintf(stderr, "mcast_discover.c inet_pton() error: %s\n", strerror(errno));
        close(sockfd);
        return -1;
    }

    if(sendto(sockfd, DISCOVER_MSG, strlen(DISCOVER_MSG), 0, (struct sockaddr*) &mcastaddr, sizeof(mcastaddr)) < 0){
        fprintf(stderr, "mcast_discover.c sendto() error: %s\n", strerror(errno));
        close(sockfd);
        return -1;
    }

    // socket delay
    struct timeval tv;
    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;
    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0){
        fprintf(stderr, "mcast_discover.c setsockopt(tv) error: %s\n", strerror(errno));
        close(sockfd);
        return -1;
    }

    while(1){
        if((n = recvfrom(sockfd, msg, (MSG_SIZE - 1), 0, (struct sockaddr*) &servaddr, &len)) < 0){
            fprintf(stderr, "mcast_discover.c recvfrom() error: %s\n", strerror(errno));
            break;
        }
        msg[n] = '\0';
        if(strcmp(msg, RESPONSE_MSG) == 0){
            inet_ntop(AF_INET6, &servaddr.sin6_addr, srvaddrstr, INET6_ADDRSTRLEN);
            printf("Server: [%s]:%d discovered\n", srvaddrstr, ntohs(servaddr.sin6_port));
        }
    }
    close(sockfd);

    return 0;
}