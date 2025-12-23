#ifndef SEND_DIRECTORY_H
#define SEND_DIRECTORY_H

#include <sys/types.h>

void send_directory(int sockfd, const char *dirpath);

#endif // SEND_DIRECTORY