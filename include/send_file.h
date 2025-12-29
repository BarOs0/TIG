#ifndef SEND_FILE_H
#define SEND_FILE_H

/*send_directory() helper, file sending function*/

#include <sys/types.h> 

#define BUFF_SIZE 1024 // File data buffer

void send_file(int sockfd, const char *filepath);

#endif // SEND_FILE