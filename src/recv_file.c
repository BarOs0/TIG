#include "recv_file.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

void recv_file(int sockfd, const char* filepath){

    int filefd;

    if((filefd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0){ // if file doesnt exist create new one, if file exists overwrite it
        perror("recv_file.c open() file error");
        return;
    }

    uint64_t filesize;
    if(read(sockfd, &filesize, sizeof(filesize)) != sizeof(filesize)){ // reading file size from peer
        perror("recv_file.c read() size error");
        close(filefd);
        return;
    }

    char buff[BUFF_SIZE];
    uint64_t bytes_left = filesize;
    ssize_t bytes_to_read;
    ssize_t bytes_read;

    while(bytes_left > 0){ // reading all bytes from file from peer
        bytes_to_read = (bytes_left > BUFF_SIZE) ? BUFF_SIZE : bytes_left; // sending 1024 bytes or less (bytes left)
        bytes_read = read(sockfd, buff, bytes_to_read); // reading file setting to a chunk
        if(bytes_read == 0) break; // if all bytes from file is read STOP
        if(bytes_read < 0){
            perror("recv_file.c read() file error");
            break;
        }  
        write(filefd, buff, bytes_read); // writing bytes to new file (consider if validation is necessery)
        bytes_left -= bytes_read; // offset adjustment
    }
    close(filefd);
}