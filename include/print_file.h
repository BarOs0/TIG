#ifndef PRINT_FILE_H
#define PRINT_FILE_H

/**
 * @file print_file.h
 * @brief A function that prints the contents of a file to the screen
 */

#define BUFF_SIZE 1024 ///< File data buffer

/**
 * @brief Prints the contents of a file to the screen
 * @param sockfd Socket file descriptor
 */
void print_file(int sockfd);

#endif // PRINT_FILE