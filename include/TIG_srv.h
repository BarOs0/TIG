#ifndef TIG_SRV_H
#define TIG_SRV_H

#include <netinet/in.h>

#define LISTENQ 2
#define LISTEN_PORT_FTP 2025
#define COMMIT_BUFF_SIZE 64
#define REPOS_BUFF_SIZE 1024
#define NAME_BUFF_SIZE 32
#define TIME_BUFF_SIZE 64
#define REPOS_PATH "/home/bartek/PS2025/projekt/srv/data"

void run(void);
void handle_client(int connfd, struct sockaddr_in6 *cliaddr);

#endif // TIG_SRV_H