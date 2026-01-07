/**
 * @file print_file.c
 * @brief Implementation of file content printing from network socket
 */

#include "print_file.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

void print_file(int sockfd){

    uint64_t filesize;
    if(read(sockfd, &filesize, sizeof(filesize)) != sizeof(filesize)){ ///< Reading file size from peer
        perror("print_file read() size error");
        return;
    }

    char buff[BUFF_SIZE + 1];
    uint64_t bytes_left = filesize;
    ssize_t bytes_to_read;
    ssize_t bytes_read;

    while(bytes_left > 0){ ///< Reading all bytes from file from peer
        bytes_to_read = (bytes_left > BUFF_SIZE) ? BUFF_SIZE : bytes_left;
        bytes_read = read(sockfd, buff, bytes_to_read); ///< Read data to the file data buffer
        if(bytes_read == 0) break;
        if(bytes_read < 0){
            perror("print_file read() file error");
            break;
        }
        buff[bytes_read] = '\0'; ///< Null-terminate for printf (safely)
        printf("%.*s", (int)bytes_read, buff); ///< Print as string even with nulls
        bytes_left -= bytes_read; ///< Decrement bytes_left
    }
    fflush(stdout); ///< Flush all through stdout
}
