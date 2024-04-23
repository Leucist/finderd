#include <time.h>
#include "setCurrentTime.h"

void setCurrentTime(char *time_str, size_t buffer_size) {
	time_t current_time;
    struct tm *local_time;
	time(&current_time);
    local_time = localtime(&current_time);
    strftime(time_str, buffer_size, "%Y-%m-%d %H:%M:%S", local_time);
}

// char *getCurrentTime() {
//     size_t size = 64;
// 	char *time_str = malloc(size);
// 	setCurrentTime(time_str, size);
// 	return time_str;
// }