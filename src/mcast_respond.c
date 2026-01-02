#include "mcast_respond.h"
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <stdio.h>

void* mcast_respond(void* arg){
    int sockfd;
    struct sockaddr_in6 mcast_addr, cli_addr;
    char msg[MSG_SIZE];
    socklen_t cli_len = sizeof(cli_addr);

    if ((sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0){ ///< Creating UDP socket to respond mcast discovery
        syslog(LOG_ERR, " mcast_respond.c socket() error: %s", strerror(errno));
        pthread_exit(NULL);
    }

    int on = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0){ ///< REUSEADDR on 
        syslog(LOG_ERR, " mcast_respond.c setsockopt(REUSEADDR) error: %s", strerror(errno));
        close(sockfd);
        pthread_exit(NULL);
    }

    bzero(&mcast_addr, sizeof(mcast_addr));
    mcast_addr.sin6_family = AF_INET6;
    mcast_addr.sin6_addr = in6addr_any;
    mcast_addr.sin6_port = htons(MCAST_PORT);

    if (bind(sockfd, (struct sockaddr*)&mcast_addr, sizeof(mcast_addr)) < 0){ ///< Bind to the mcast port (default 2026)
        syslog(LOG_ERR, " mcast_respond.c bind() error: %s", strerror(errno));
        close(sockfd);
        pthread_exit(NULL);
    }

    syslog(LOG_INFO, " mcast_respond.c waiting for discovery MSG on: %s\n", MCAST_IF); ///< LOGGING FOR ADMINISTRATOR

    struct ipv6_mreq mreq; ///< Prepare IPv6 mreq structure for multicast
    if (inet_pton(AF_INET6, MCAST_ADDR, &mreq.ipv6mr_multiaddr) != 1){
        syslog(LOG_ERR, " mcast_respond.c inet_pton() error");
        close(sockfd);
        pthread_exit(NULL);
    }

    mreq.ipv6mr_interface = if_nametoindex(MCAST_IF); ///< Get interface index
    if(mreq.ipv6mr_interface == 0){
        syslog(LOG_ERR, " mcast_respond.c interface %s unreachable", MCAST_IF);
        close(sockfd);
        pthread_exit(NULL);
    }

    if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq, sizeof(mreq)) < 0){ ///< Join mcast group with ff02::2026
        syslog(LOG_ERR, " mcast_respond.c setsockopt(IPV6_JOIN_GROUP) error: %s", strerror(errno));
        close(sockfd);
        pthread_exit(NULL);
    }

    /** PREPARING RESPONSE MESSAGE */ 

    struct ifaddrs *ifaddr, *ifa; 
    char addr[INET6_ADDRSTRLEN];

    if (getifaddrs(&ifaddr) == -1) {
        syslog(LOG_ERR, " getifaddrs() error: %s", strerror(errno));
        close(sockfd);
        pthread_exit(NULL);
    }

    char addrstr[ADDR_BUFF_SIZE] = {0};
    strcpy(addrstr, "Interface: ");
    strcat(addrstr, MCAST_IF);
    strcat(addrstr, " IPv6 server addresses:\n");
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) { ///< Check MCAST_IF IPv6 addresses, and build response
        if (ifa->ifa_addr &&
            ifa->ifa_addr->sa_family == AF_INET6 &&
            strcmp(ifa->ifa_name, MCAST_IF) == 0) {
            struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)ifa->ifa_addr;
            if (inet_ntop(AF_INET6, &sin6->sin6_addr, addr, sizeof(addr))) {
                strcat(addrstr, addr);
                strcat(addrstr, "\n");
            }
        }
    }
    freeifaddrs(ifaddr);

    /** SENDING RESPONSE */ 

    int n;
    while (1){
        if((n = recvfrom(sockfd, msg, (MSG_SIZE - 1), 0, (struct sockaddr*)&cli_addr, &cli_len)) <0){ ///< Receive discovery message
            syslog(LOG_ERR, " mcast_respond.c recvfrom() error: %s", strerror(errno));
            continue;
        }
        msg[n] = '\0';
        syslog(LOG_INFO, " mcast_respond.c recieved discovery MSG: %s\n", msg); ///< LOGGING FOR ADMINISTRATOR
        if (strcmp(msg, DISCOVER_MSG) == 0){ ///< Check if it is TIG mcast discover
            sendto(sockfd, addrstr, strlen(addrstr), 0, (struct sockaddr*)&cli_addr, cli_len); ///< Send mcast discovery response
            syslog(LOG_INFO, " mcast_respond.c sent discovery response."); ///< LOGGING FOR ADMINISTRATOR
        }
    }
    close(sockfd);
    pthread_exit(NULL);
}