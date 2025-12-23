#include "recv_file.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

void recv_file(int sockfd, const char* filepath){
    int filefd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0644); // if file doesnt exist create new one, if file exists overwrite it
    if(filefd < 0){
        return;
    }

    char buff[BUFF_SIZE];
    ssize_t bytes_read, bytes_written;

    while((bytes_read = read(sockfd, buff, BUFF_SIZE)) > 0){
        ssize_t total_bytes_written = 0;
        while(total_bytes_written < bytes_read){
            bytes_written = write(filefd, (buff + total_bytes_written), (bytes_read - total_bytes_written));
            if(bytes_written < 0){
                close(filefd);
                return;
            }
            total_bytes_written += bytes_written;
        }
    }
    close(filefd);
}