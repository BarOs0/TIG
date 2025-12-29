#ifndef TIG_SRV_H
#define TIG_SRV_H

#include <netinet/in.h>

#define LISTENQ 2 // Clients queue for listen()
#define LISTEN_PORT_FTP 2025 // Listening port
#define COMMIT_BUFF_SIZE 64 // Commit message buffer
#define NAME_BUFF_SIZE 32 // File or directory name buffer
#define TIME_BUFF_SIZE 64 // Time stamp buffer
#define REPOS_PATH "/home/bartek/PS2025/projekt/srv/data" // Server data directory path (user configurable)

void run(void); // Main server initialization function
void handle_client(int connfd, struct sockaddr_in6 *cliaddr); // Client handle function
void sigchld_handler(int signo); // SIGCHLD handler

#endif // TIG_SRV_H