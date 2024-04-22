#ifndef SET_CURRENT_TIME_H
#define SET_CURRENT_TIME_H

/**
 * Sets the current time in the specified buffer string.
 * 
 * Parameters:
 *   current_time: Pointer to a time_t variable to store the current time.
 *   local_time: Pointer to a struct tm variable to store the local time.
 *   time_str: Pointer to a char buffer where the formatted time string will be stored.
 *   buffer_size: Size of the time_str buffer.
 * 
 * Returns:
 *   None.
 */

void setCurrentTime(time_t *current_time, struct tm *local_time, char* time_str, size_t buffer_size);

#endif