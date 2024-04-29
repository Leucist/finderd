#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// #include <sys/wait.h>
#include <sys/stat.h>
// #include <signal.h>
#include <syslog.h>

#include "search.h"
#include "setCurrentTime.h"

void search(const char *path, const char *keyword, int *anyFilesFound, int verbose) {
    DIR *dir;
    struct dirent *entry;		// starting directory
    struct stat statbuf;		// current directory
    char fullPath[PATH_MAX];	// structure that hold file attributes
    char time_str[64];			// string to contain current time
    char message[256];			// message to be sent to the syslog

    if ((dir = opendir(path)) == NULL) { // open starting directory and return in case of errors
        if(verbose){			// if we have -v parameter
        	setCurrentTime(time_str, sizeof(time_str));
			sprintf(message, "%s | Failed to open directory", time_str);
			syslog(LOG_INFO, "%s", message);
        }
        closedir(dir);
        return;
    }

    while ((entry = readdir(dir)) != NULL) { // iterate by each file and folder from the starting directory
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) // skip current and parent directory
            continue;

        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry->d_name);


   //      if (verbose) {
   //      	setCurrentTime(time_str, sizeof(time_str));
			// sprintf(message, "%s | Checking file: %s, Pattern: %s", time_str, fullPath, keyword);
			// syslog(LOG_INFO, "%s", message);
   //      }
        

        
        if (lstat(fullPath, &statbuf) != 0) { // checks if entry cannot be accessed
        	if(verbose) {
        		setCurrentTime(time_str, sizeof(time_str));
				sprintf(message, "%s | %s is not accessible", time_str, fullPath);
				syslog(LOG_INFO, "%s", message);
        	}
            continue;		// skip to the next iteration
        }

        // if (S_ISREG(statbuf.st_mode)) {	// checks if entry is a regular file
            
        // }
        // char* entryType = 

        if (strstr(entry->d_name, keyword) != NULL) { // compare the entry name to searching file name
        	setCurrentTime(time_str, sizeof(time_str));
			sprintf(message, "%s | Found: %s, Pattern: %s", time_str, fullPath, keyword);
			syslog(LOG_INFO, "%s", message);
			*anyFilesFound = 1;
        }

        else if (S_ISDIR(statbuf.st_mode)) { // checks if entry is a directory
            search(fullPath, keyword, anyFilesFound, verbose);
        }
    }

    closedir(dir);
}