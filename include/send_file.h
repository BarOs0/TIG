#ifndef SEND_FILE_H
#define SEND_FILE_H

/**
 * @file send_file.h
 * @brief send_directory() helper, file sending function
 */

#include <sys/types.h> 

#define BUFF_SIZE 1024 ///< File data buffer

/**
 * @brief File sending function
 * @param sockfd Socket file descriptor
 * @param filepath File path
 */
void send_file(int sockfd, const char *filepath);

#endif // SEND_FILE