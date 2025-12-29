#include "mcast_respond.h"
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <net/if.h>

void* mcast_respond(void* arg){
    int sockfd;
    struct sockaddr_in6 mcast_addr, cli_addr;
    char msg[MSG_SIZE];
    socklen_t cli_len = sizeof(cli_addr);

    if ((sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0){
        syslog(LOG_ERR, "mcast_responder.c socket() error: %s", strerror(errno));
        pthread_exit(NULL);
    }

    int on = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0){
        syslog(LOG_ERR, "mcast_responder.c setsockopt(REUSEADDR) error: %s", strerror(errno));
        close(sockfd);
        pthread_exit(NULL);
    }

    bzero(&mcast_addr, sizeof(mcast_addr));
    mcast_addr.sin6_family = AF_INET6;
    mcast_addr.sin6_addr = in6addr_any;
    mcast_addr.sin6_port = htons(MCAST_PORT);

    if (bind(sockfd, (struct sockaddr*)&mcast_addr, sizeof(mcast_addr)) < 0){
        syslog(LOG_ERR, "mcast_responder.c bind() error: %s", strerror(errno));
        close(sockfd);
        pthread_exit(NULL);
    }

    syslog(LOG_INFO, "mcast_responder.c waiting for discovery MSG on: %s\n", MCAST_IF);

    struct ipv6_mreq mreq;
    if (inet_pton(AF_INET6, MCAST_ADDR, &mreq.ipv6mr_multiaddr) != 1){
        syslog(LOG_ERR, "mcast_responder.c inet_pton() error");
        close(sockfd);
        pthread_exit(NULL);
    }

    mreq.ipv6mr_interface = if_nametoindex(MCAST_IF);
    if(mreq.ipv6mr_interface == 0){
        syslog(LOG_ERR, "mcast_responder.c interface %s unreachable", MCAST_IF);
        close(sockfd);
        pthread_exit(NULL);
    }

    if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq, sizeof(mreq)) < 0){
        syslog(LOG_ERR, "mcast_responder.c setsockopt(IPV6_JOIN_GROUP) error: %s", strerror(errno));
        close(sockfd);
        pthread_exit(NULL);
    }

    while (1){
        int n = recvfrom(sockfd, msg, (MSG_SIZE - 1), 0, (struct sockaddr*)&cli_addr, &cli_len);
        if (n < 0){
            syslog(LOG_ERR, "mcast_responder.c recvfrom() error: %s", strerror(errno));
            continue;
        }
        msg[n] = '\0';
        syslog(LOG_INFO, "mcast_respond.c recieved discovery MSG: %s\n", msg);
        if (strcmp(msg, DISCOVER_MSG) == 0){
            sendto(sockfd, RESPONSE_MSG, strlen(RESPONSE_MSG), 0, (struct sockaddr*)&cli_addr, cli_len);
            syslog(LOG_INFO, "mcast_respond.c sent discovery response: %s\n", RESPONSE_MSG);
        }
    }
    close(sockfd);
    pthread_exit(NULL);
}