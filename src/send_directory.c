#include "send_directory.h"
#include "send_file.h"
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>

void send_directory(int sockfd, const char* dirpath){
    DIR *dir = opendir(dirpath);
    struct dirent *entry; // single file or directory
    char filepath[PATH_MAX];
    char type;

    while((entry = readdir(dir)) != NULL){
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){
            continue;
        }
        if(strcmp(entry->d_name, "TIG_cli") == 0){ // skip binary file TIG_cli
            continue;
        }
        snprintf(filepath, PATH_MAX, "%s/%s", dirpath, entry->d_name); // prepare directory path

        struct stat st; // struct for stat
        stat(filepath, &st); // details for filepath (purpose, type, ...)
        if(S_ISDIR(st.st_mode)){ // is it directory?
            type = 'D';
            write(sockfd, &type, 1); // send D
            write(sockfd, entry->d_name, strlen(entry->d_name) + 1); // send directory name
            send_directory(sockfd, filepath); // if directory call this function again
        }
        else if(S_ISREG(st.st_mode)){ // is it file?
            type = 'F';
            write(sockfd, &type, 1); // send F
            write(sockfd, entry->d_name, strlen(entry->d_name) + 1); // send file name
            send_file(sockfd, filepath); // use helper function (send file)
        }
    }
    type = 'E'; // the end of directory
    write(sockfd, &type, 1); // send E 
    closedir(dir);
}