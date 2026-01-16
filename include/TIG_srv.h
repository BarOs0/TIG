#ifndef TIG_SRV_H
#define TIG_SRV_H

/**
 * @file TIG_srv.h
 * @brief TIG server main header file
 */

#include <netinet/in.h>

#define LISTENQ 2 ///< Clients queue for listen()
#define LISTEN_PORT_FTP 2025 ///< Listening port
#define COMMIT_BUFF_SIZE 64 ///< Commit message buffer
#define NAME_BUFF_SIZE 32 ///< File or directory name buffer
#define TIME_BUFF_SIZE 64 ///< Time stamp buffer
#define DATA_PATH "/home/bartek/PS2025/projekt/srv/data" ///< Server data directory path (administrator configurable)

/**
 * @brief Main server initialization function
 */
void run(void);

/**
 * @brief Client handle function
 * @param connfd Connection file descriptor
 * @param cliaddr Client address structure
 */
void handle_client(int connfd, struct sockaddr_in6 *cliaddr);

/**
 * @brief SIGCHLD handler
 * @param signo Signal number
 */
void sigchld_handler(int signo);

/**
 * @brief File lock/unlock functions for fork()
 * @param fd file descriptor
 */
int lock_file_write(int fd);
int lock_file_read(int fd);
int unlock_file(int fd);

#endif // TIG_SRV_H