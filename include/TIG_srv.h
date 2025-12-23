#ifndef TIG_SRV_H
#define TIG_SRV_H

#include <netinet/in.h>

#define LISTENQ 2
#define LISTEN_PORT_FTP 2025

void run(void);
void handle_client(int connfd, struct sockaddr_in6 *cliaddr);

#endif // TIG_SRV_H