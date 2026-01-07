/**
 * @file mcast_discover.c
 * @brief Implementation of IPv6 multicast discovery for server detection
 */

#include "mcast_discover.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <net/if.h>

int mcast_discover(void){

    int sockfd, n;
    struct sockaddr_in6 mcastaddr, servaddr;

    bzero(&mcastaddr, sizeof(mcastaddr));
    bzero(&servaddr, sizeof(servaddr));
    socklen_t len = sizeof(servaddr);

    char srvaddrstr[ADDR_BUFF_SIZE] = {0};

    if((sockfd = socket(AF_INET6, SOCK_DGRAM,0)) < 0){ ///< UDP socket for multicast discovery 
        fprintf(stderr, "mcast_discover.c socket() error: %s\n", strerror(errno));
        return -1;
    }

    int ifindex;
    if((ifindex = if_nametoindex(MCAST_IF)) == 0){ ///< Get interface index (user may configure this one)
        fprintf(stderr, "mcast_discover.c interface: %s unreachable\n", MCAST_IF);
        close(sockfd);
        return -1;
    }

    if(setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_IF, &ifindex, sizeof(ifindex)) < 0){ ///< Send multicast discovery via user-configurable interface
        fprintf(stderr, "mcast_discover.c setsockopt(ifindex) error: %s\n", strerror(errno));
        close(sockfd);
        return -1;
    }

    mcastaddr.sin6_family = AF_INET6; ///< Prepare multicast sendto() address
    mcastaddr.sin6_port = htons(MCAST_PORT);

    if(inet_pton(AF_INET6, MCAST_ADDR, &mcastaddr.sin6_addr) < 0){ 
        fprintf(stderr, "mcast_discover.c inet_pton() error: %s\n", strerror(errno));
        close(sockfd);
        return -1;
    }

    if(sendto(sockfd, DISCOVER_MSG, strlen(DISCOVER_MSG), 0, (struct sockaddr*) &mcastaddr, sizeof(mcastaddr)) < 0){ ///< Send multicast
        fprintf(stderr, "mcast_discover.c sendto() error: %s\n", strerror(errno));
        close(sockfd);
        return -1;
    }

    struct timeval tv; ///< Socket delay (default 3s)
    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;
    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0){
        fprintf(stderr, "mcast_discover.c setsockopt(tv) error: %s\n", strerror(errno));
        close(sockfd);
        return -1;
    }

    if((n = recvfrom(sockfd, srvaddrstr, (ADDR_BUFF_SIZE - 1), 0, (struct sockaddr*) &servaddr, &len)) < 0){ ///< Receive response from server
        fprintf(stderr, "mcast_discover.c recvfrom() error: %s\n", strerror(errno));
        close(sockfd);
        return -1;
    }
    srvaddrstr[n] = '\0';
    printf("%s", srvaddrstr); ///< Print the address found by multicast discovery 
    close(sockfd);

    return 0;
}