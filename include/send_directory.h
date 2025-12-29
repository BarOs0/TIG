#ifndef SEND_DIRECTORY_H
#define SEND_DIRECTORY_H

/*A function that recursively sends the entire directory*/

#include <sys/types.h>

void send_directory(int sockfd, const char *dirpath);

#endif // SEND_DIRECTORY