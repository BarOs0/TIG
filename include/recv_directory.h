#ifndef RECV_DIRECTORY_H
#define RECV_DIRECTORY_H

/*A function that recursively retrieves the entire directory - overwrites or creates directories*/

#include <sys/types.h>

#define FILE_NAME_SIZE 128

void clear_dir(const char* path); // recv_directory() helper - for overwriting
void recv_directory(int sockfd, const char *dirpath);

#endif // RECV_DIRECTORY