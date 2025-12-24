#ifndef RECV_DIRECTORY_H
#define RECV_DIRECTORY_H

#include <sys/types.h>

#define FILE_NAME_SIZE 128

void clear_dir(const char* path);
void recv_directory(int sockfd, const char *dirpath);

#endif // RECV_DIRECTORY