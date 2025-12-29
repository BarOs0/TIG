#ifndef COPY_DIRECTORY_H
#define COPY_DIRECTORY_H

/*A function that recursively copies an entire directory*/

#define BUFF_SIZE 1024

void copy_file(const char* srcpath, const char* dstpath); // copy_directory() helper
void copy_directory(const char* srcdir, const char* dstdir);

#endif // COPY_DIRECTORY