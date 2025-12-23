#include "send_file.h"
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

void send_file(int sockfd, const char* filepath){

    int filefd;

    if((filefd = open(filepath, O_RDONLY)) < 0){ // opening a file
        return;
    }

    char buff[BUFF_SIZE];
    ssize_t bytes_read, bytes_sent;

    while((bytes_read = read(filefd, buff, BUFF_SIZE)) > 0){ // reading this file
        ssize_t total_bytes_sent = 0; // sent bytes counter
        while(total_bytes_sent < bytes_read){ // if whole file is sent stop sending
            if((bytes_sent = write(sockfd, (buff + total_bytes_sent), (bytes_read - total_bytes_sent))) < 0){ // keep sending bytes
                /*
                (buff + total_bytes_sent) = pointer to a location in the buffer from which you still have something to send
                (bytes_read - total_bytes_sent) = is the number of bytes left to be sent from the current buffer
                */
                close(filefd);
                return;
            }
            total_bytes_sent += bytes_sent; // increment
        }
    }
    close(filefd);
}