#ifndef GET_TIME_H
#define GET_TIME_H

/*A function that returns the current time in the format: Y-M-D H:M:S*/

#include <unistd.h>

void get_time(char* time_buff, size_t size);

#endif // GET_TIME