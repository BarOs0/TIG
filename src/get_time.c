#include "get_time.h"
#include <time.h>
#include <sys/time.h>

void get_time(char* time_buff, size_t size){
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(time_buff, size, "%Y-%m-%d %H:%M:%S", tm_info);
}