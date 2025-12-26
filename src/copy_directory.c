#include "copy_directory.h"
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

void copy_file(const char* srcpath, const char* dstpath){
    int srcfd = open(srcpath, O_RDONLY);
    if(srcfd < 0) return;
    int dstfd = open(dstpath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(dstfd < 0){ close(srcfd); return; }
    char buff[BUFF_SIZE];
    ssize_t bytes_read;
    while((bytes_read = read(srcfd, buff, sizeof(buff))) > 0){
        write(dstfd, buff, bytes_read);
    }
    close(srcfd);
    close(dstfd);
}

void copy_directory(const char* srcdir, const char* dstdir){
    struct stat st;
    if(stat(dstdir, &st) != 0){
        mkdir(dstdir, 0755);
    }

    DIR* dir = opendir(srcdir);
    if(dir == NULL) return;
    struct dirent* entry;
    char srcpath[PATH_MAX];
    char dstpath[PATH_MAX];

    while((entry = readdir(dir)) != NULL){
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){
            continue;
        }
        snprintf(srcpath, PATH_MAX, "%s/%s", srcdir, entry->d_name);
        snprintf(dstpath, PATH_MAX, "%s/%s", dstdir, entry->d_name);

        if(stat(srcpath, &st) == 0){
            if(S_ISDIR(st.st_mode)){
                copy_directory(srcpath, dstpath);
            }
            else if(S_ISREG(st.st_mode)){
                copy_file(srcpath, dstpath);
            }
        }
    }
    closedir(dir);
}