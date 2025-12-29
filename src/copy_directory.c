#include "copy_directory.h"
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

void copy_file(const char* srcpath, const char* dstpath){
    int srcfd = open(srcpath, O_RDONLY); // open file, read only
    if(srcfd < 0) return;
    int dstfd = open(dstpath, O_WRONLY | O_CREAT | O_TRUNC, 0644); // open file, writable, if it does not exsist create new one
    if(dstfd < 0){ 
        close(srcfd); 
        return; 
    }
    char buff[BUFF_SIZE];
    ssize_t bytes_read;
    while((bytes_read = read(srcfd, buff, sizeof(buff))) > 0){ // read from source file
        write(dstfd, buff, bytes_read); // write to destination file 
    }
    close(srcfd);
    close(dstfd);
}

void copy_directory(const char* srcdir, const char* dstdir){
    struct stat st;
    if(stat(dstdir, &st) != 0){ // check if directory exists
        mkdir(dstdir, 0755); // if not create a new one 
    }

    DIR* dir = opendir(srcdir); // open directory from source path
    if(dir == NULL) return;
    struct dirent* entry;
    char srcpath[PATH_MAX];
    char dstpath[PATH_MAX];

    while((entry = readdir(dir)) != NULL){ // iterate through directory content
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){ // skip . or .. dierctories
            continue;
        }
        snprintf(srcpath, PATH_MAX, "%s/%s", srcdir, entry->d_name); // prepare soruce path
        snprintf(dstpath, PATH_MAX, "%s/%s", dstdir, entry->d_name); // prepare destination path

        if(stat(srcpath, &st) == 0){
            if(S_ISDIR(st.st_mode)){ // if directory
                copy_directory(srcpath, dstpath); // recursive copying
            }
            else if(S_ISREG(st.st_mode)){ // if file
                copy_file(srcpath, dstpath);
            }
        }
    }
    closedir(dir);
}