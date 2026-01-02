#ifndef SEND_DIRECTORY_H
#define SEND_DIRECTORY_H

/**
 * @file send_directory.h
 * @brief A function that recursively sends the entire directory
 */

#include <sys/types.h>

/**
 * @brief Recursively sends the entire directory
 * @param sockfd Socket file descriptor
 * @param dirpath Directory path
 */
void send_directory(int sockfd, const char *dirpath);

#endif // SEND_DIRECTORY