#include "TIG_cli.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int connection() {
    int n = 0, sockfd = 0;
    char buff[BUFF_SIZE];
    struct addrinfo hints, *res, *rp;
    int err;
    const char *server_name = SERVER_NAME;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;

    char portstr[16];
    snprintf(portstr, sizeof(portstr), "%d", LISTEN_PORT_FTP);

    if ((err = getaddrinfo(server_name, portstr, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo() error for: %s\n", gai_strerror(err));
        return -1;
    }

    for (rp = res; rp != NULL; rp = rp->ai_next) {
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd == -1)
            continue;

        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;

        close(sockfd);
    }

    if (rp == NULL) {
        fprintf(stderr, "Could not connect to %s\n", server_name);
        freeaddrinfo(res);
        return -1;
    }

    while ((n = read(sockfd, buff, BUFF_SIZE-1)) > 0) {
        buff[n] = 0;
        if (fputs(buff, stdout) == EOF) {
            fprintf(stderr, "fputs() error: %s\n", strerror(errno));
            freeaddrinfo(res);
            close(sockfd);
            return -1;
        }
    }

    if (n < 0)
        fprintf(stderr, "read error : %s\n", strerror(errno));

    fflush(stdout);
    fprintf(stderr, "\nOK\n");
    freeaddrinfo(res);
    close(sockfd);

    return 0;
}

int main(int argc, char **argv){
    return connection();
}