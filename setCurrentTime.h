#ifndef SET_CURRENT_TIME_H
#define SET_CURRENT_TIME_H

/**
 * Sets the current time in the specified buffer string.
 * 
 * Parameters:
 *   time_str: Pointer to a char buffer where the formatted time string will be stored.
 *   buffer_size: Size of the time_str buffer.
 * 
 * Returns:
 *   None.
 */
void setCurrentTime(char* time_str, size_t buffer_size);

// char *getCurrentTime();

#endif