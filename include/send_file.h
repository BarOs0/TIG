#ifndef SEND_FILE_H
#define SEND_FILE_H

#include <sys/types.h>

#define BUFF_SIZE 1024

void send_file(int sockfd, const char *filepath);

#endif // SEND_FILE