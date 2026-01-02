#ifndef GET_TIME_H
#define GET_TIME_H

/**
 * @file get_time.h
 * @brief A function that returns the current time in the format: Y-M-D H:M:S
 */

#include <unistd.h>

/**
 * @brief Returns the current time in the format: Y-M-D H:M:S
 * @param time_buff Buffer to store the formatted time
 * @param size Size of the buffer
 */
void get_time(char* time_buff, size_t size);

#endif // GET_TIME