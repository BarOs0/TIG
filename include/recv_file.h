#ifndef RECV_FILE_H
#define RECV_FILE_H

/**
 * @file recv_file.h
 * @brief recv_directory() helper, file receiving function - overwrites or creates files
 */

#include <sys/types.h>

#define BUFF_SIZE 1024 ///< File data buffer

/**
 * @brief File receiving function - overwrites or creates files
 * @param sockfd Socket file descriptor
 * @param filepath File path
 */
void recv_file(int sockfd, const char *filepath);

#endif // RECV_FILE