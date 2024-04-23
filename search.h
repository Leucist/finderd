#ifndef SEARCH_H
#define SEARCH_H

/**
 * Recursively searches for files containing the specified fragment in their name
 * within the given directory and its subdirectories.
 *
 * Parameters:
 *    directory: The directory to start the search from.
 *    keyword: The keyword to search for in file names.
 *    anyFilesFound: The indicator whether any files were found (1) or not (0).
 *    verbose: The indicator whether notifications are enabled (1) or not (0).
 *
 * Returns:
 *    None.
 */
void search(const char* path, const char* keyword, int* anyFilesFound, int verbose);

#endif
