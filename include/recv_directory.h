#ifndef RECV_DIRECTORY_H
#define RECV_DIRECTORY_H

#include <sys/types.h>

#define FILE_NAME_SIZE 128 ///< File or directory name buffer

/**
 * @brief recv_directory() helper - for overwriting, this function deletes directory content recursively
 * @param path Directory path
 */
void clear_dir(const char* path);

/**
 * @brief Recursively retrieves the entire directory - overwrites or creates directories
 * @param sockfd Socket file descriptor
 * @param dirpath Directory path
 */
void recv_directory(int sockfd, const char *dirpath);

#endif // RECV_DIRECTORY