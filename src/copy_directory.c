/**
 * @file copy_directory.c
 * @brief Implementation of recursive directory and file copying
 */

#include "copy_directory.h"
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

void copy_file(const char* srcpath, const char* dstpath){
    int srcfd = open(srcpath, O_RDONLY); ///< Open file, read only
    if(srcfd < 0) return;
    int dstfd = open(dstpath, O_WRONLY | O_CREAT | O_TRUNC, 0644); ///< Open file, writable, if it does not exist create new one
    if(dstfd < 0){ 
        close(srcfd); 
        return; 
    }
    char buff[BUFF_SIZE];
    ssize_t bytes_read;
    while((bytes_read = read(srcfd, buff, sizeof(buff))) > 0){ ///< Read from source file
        write(dstfd, buff, bytes_read); ///< Write to destination file 
    }
    close(srcfd);
    close(dstfd);
}

void copy_directory(const char* srcdir, const char* dstdir){
    struct stat st;
    if(stat(dstdir, &st) != 0){ ///< Check if directory exists
        mkdir(dstdir, 0755); ///< If not create a new one 
    }

    DIR* dir = opendir(srcdir); ///< Open directory from source path
    if(dir == NULL) return;
    struct dirent* entry;
    char srcpath[PATH_MAX];
    char dstpath[PATH_MAX];

    while((entry = readdir(dir)) != NULL){ ///< Iterate through directory content
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){ ///< Skip . or .. directories
            continue;
        }
        snprintf(srcpath, PATH_MAX, "%s/%s", srcdir, entry->d_name); ///< Prepare source path
        snprintf(dstpath, PATH_MAX, "%s/%s", dstdir, entry->d_name); ///< Prepare destination path

        if(stat(srcpath, &st) == 0){
            if(S_ISDIR(st.st_mode)){ ///< If directory
                copy_directory(srcpath, dstpath); ///< Recursive copying
            }
            else if(S_ISREG(st.st_mode)){ ///< If file
                copy_file(srcpath, dstpath);
            }
        }
    }
    closedir(dir);
}