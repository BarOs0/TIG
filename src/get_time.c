#include "get_time.h"
#include <time.h>
#include <sys/time.h>

void get_time(char* time_buff){
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(time_buff, TIME_BUFF_SIZE, "%Y-%m-%d %H:%M:%S", tm_info);
}