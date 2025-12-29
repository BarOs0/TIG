#ifndef RECV_FILE_H
#define RECV_FILE_H

/*recv_directory() helper, file receiving function - overwrites or creates files*/

#include <sys/types.h>

#define BUFF_SIZE 1024 // File data buffer

void recv_file(int sockfd, const char *filepath);

#endif // RECV_FILE