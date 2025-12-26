#ifndef COPY_DIRECTORY_H
#define COPY_DIRECTORY_H

#define BUFF_SIZE 1024

void copy_file(const char* srcpath, const char* dstpath);
void copy_directory(const char* srcdir, const char* dstdir);

#endif // COPY_DIRECTORY