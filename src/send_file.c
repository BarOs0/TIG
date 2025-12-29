#include "send_file.h"
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

void send_file(int sockfd, const char* filepath){

    /*
        ====SENDING FILE SIZE====
    */

    struct stat st; // structure with statistics of file
    if (stat(filepath, &st) < 0){
        perror("send_file.c stat() error");
        return;
    }

    off_t filesize = st.st_size; // off_t - file size (bytes)

    uint64_t size_to_send = (uint64_t)filesize;
    if (write(sockfd, &size_to_send, sizeof(size_to_send)) != sizeof(size_to_send)){ // sending file size
        perror("send_file.c write() size error");
        return;
    }

    /*
        ====SENDING FILE====
    */

    int filefd;

    if((filefd = open(filepath, O_RDONLY)) < 0){ // opening a file
        perror("send_file.c open() file error");
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
                perror("send_file.c write() file error");
                return;
            }
            total_bytes_sent += bytes_sent; // increment
        }
    }
    close(filefd);
}