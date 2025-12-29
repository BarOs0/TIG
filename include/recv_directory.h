#ifndef RECV_DIRECTORY_H
#define RECV_DIRECTORY_H

#include <sys/types.h>

#define FILE_NAME_SIZE 128 // File or directory name buffer

void clear_dir(const char* path); // recv_directory() helper - for overwriting, this function delates directory content recursively
void recv_directory(int sockfd, const char *dirpath); // A function that recursively retrieves the entire directory - overwrites or creates directories

#endif // RECV_DIRECTORY