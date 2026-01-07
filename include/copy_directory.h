#ifndef COPY_DIRECTORY_H
#define COPY_DIRECTORY_H

/**
 * @file copy_directory.h
 * @brief Functions for recursively copying directories and files
 */

#define BUFF_SIZE 1024 ///< File data buffer

/**
 * @brief Helper function for copy_directory()
 * @param srcpath Source file path
 * @param dstpath Destination file path
 */
void copy_file(const char* srcpath, const char* dstpath);

/**
 * @brief Recursively copies an entire directory
 * @param srcdir Source directory path
 * @param dstdir Destination directory path
 */
void copy_directory(const char* srcdir, const char* dstdir);

#endif // COPY_DIRECTORY