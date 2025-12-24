#include "recv_file.h"
#include "recv_directory.h"
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>

void clear_dir(const char* dirpath){
    DIR* dir = opendir(dirpath); // open directory
    if(dir == NULL) return;
    struct dirent* entry;
    char filepath[PATH_MAX];

    while((entry = readdir(dir)) != NULL){
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){
            continue;
        }
        snprintf(filepath, PATH_MAX, "%s/%s", dirpath, entry->d_name);
        struct stat st;
        if(stat(filepath, &st) == 0){
            if(S_ISDIR(st.st_mode)){
                clear_dir(filepath); // recursive clearing
                rmdir(filepath); // if sub-diercotry is empty move to trash
            }
            else if(S_ISREG(st.st_mode)){
                unlink(filepath); // if file delete it
            }
        }
    }
    closedir(dir);
}

void recv_directory(int sockfd, const char* dirpath){
    struct stat st;
    if(stat(dirpath, &st) == 0 && S_ISDIR(st.st_mode)){
        clear_dir(dirpath);
    }
    else{
        mkdir(dirpath, 0755);
    }

    char type;
    char name[FILE_NAME_SIZE];
    int i = 0;
    char c;
    char path[PATH_MAX];

    while(1){

        if(read(sockfd, &type, 1) != 1){
            perror("recv_directory.c read() invalid prefix name");
            return;
        }

        if(type == 'E') break;

        i = 0;
        do{
            if(read(sockfd, &c, 1) != 1){
                perror("recv_directory.c read name error");
                return;
            }
            name[i++] = c;
        }while(c != '\0' && i < FILE_NAME_SIZE);

        if(i == FILE_NAME_SIZE){
            perror("Maximum file name is too low");
            return;
        }

        name[FILE_NAME_SIZE-1] = '\0';

        snprintf(path, PATH_MAX, "%s/%s", dirpath, name);

        switch(type){ 

            case 'D':
                recv_directory(sockfd, path);
                break;
            
            case 'F':
                recv_file(sockfd, path);
                break;

            default:
                return;
        }
    }
}