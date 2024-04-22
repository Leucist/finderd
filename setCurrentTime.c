#include <time.h>
#include "setCurrentTime.h"

void setCurrentTime(time_t *current_time, struct tm *local_time, char* time_str, size_t buffer_size) {
	time(current_time);
    local_time = localtime(current_time);
    strftime(time_str, buffer_size, "%Y-%m-%d %H:%M:%S", local_time);
}
