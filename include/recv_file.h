#ifndef RECV_FILE_H
#define RECV_FILE_H

#include <sys/types.h>

#define BUFF_SIZE 1024

void recv_file(int sockfd, const char *filepath);

#endif // RECV_FILE