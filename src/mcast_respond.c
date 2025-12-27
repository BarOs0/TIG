#include "mcast_respond.h"
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

void* mcast_responder(void* arg){
    int sockfd;
    struct sockaddr_in6 mcast_addr, cli_addr;
    char buff[BUFF_SIZE];
    socklen_t cli_len = sizeof(cli_addr);

    if ((sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0){
        syslog(LOG_ERR, "mcast_responder socket() error: %s", strerror(errno));
        pthread_exit(NULL);
    }

    int on = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0){
        syslog(LOG_ERR, "mcast_responder setsockopt(REUSEADDR) error: %s", strerror(errno));
        close(sockfd);
        pthread_exit(NULL);
    }

    memset(&mcast_addr, 0, sizeof(mcast_addr));
    mcast_addr.sin6_family = AF_INET6;
    mcast_addr.sin6_addr = in6addr_any;
    mcast_addr.sin6_port = htons(MCAST_PORT);

    if (bind(sockfd, (struct sockaddr*)&mcast_addr, sizeof(mcast_addr)) < 0){
        syslog(LOG_ERR, "mcast_responder bind() error: %s", strerror(errno));
        close(sockfd);
        pthread_exit(NULL);
    }

    struct ipv6_mreq mreq;
    if (inet_pton(AF_INET6, MCAST_ADDR, &mreq.ipv6mr_multiaddr) != 1){
        syslog(LOG_ERR, "mcast_responder inet_pton() error");
        close(sockfd);
        pthread_exit(NULL);
    }
    mreq.ipv6mr_interface = 0;

    if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq, sizeof(mreq)) < 0){
        syslog(LOG_ERR, "mcast_responder setsockopt(IPV6_JOIN_GROUP) error: %s", strerror(errno));
        close(sockfd);
        pthread_exit(NULL);
    }

    while (1){
        int n = recvfrom(sockfd, buff, (BUFF_SIZE - 1), 0, (struct sockaddr*)&cli_addr, &cli_len);
        if (n < 0){
            syslog(LOG_ERR, "mcast_responder recvfrom() error: %s", strerror(errno));
            continue;
        }
        buff[n] = '\0';
        if (strcmp(buff, DISCOVER_MSG) == 0){
            sendto(sockfd, RESPONSE_MSG, strlen(RESPONSE_MSG), 0, (struct sockaddr*)&cli_addr, cli_len);
        }
    }
    close(sockfd);
    pthread_exit(NULL);
}