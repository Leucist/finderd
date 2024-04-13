#ifndef SEARCH_H
#define SEARCH_H

/**
 * Recursively searches for files containing the specified fragment in their name
 * within the given directory and its subdirectories.
 *
 * @param directory The directory to start the search from
 * @param fragment The fragment to search for in file names
 * @return the count of located files, -1 on failure
 */
int search(char* directory, char* fragment);

#endif
