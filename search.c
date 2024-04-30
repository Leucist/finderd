#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "search.h"
#include "sendLog.h"

void search(const char *path, const char *keyword, int *anyFilesFound, int verbose) {
    DIR *dir;
    struct dirent *entry;		// starting directory
    struct stat statbuf;		// current directory
    char fullPath[PATH_MAX];	// structure that hold file attributes
    char message[256];			// message to be sent to the SYSLOG

    if ((dir = opendir(path)) == NULL) { // open starting directory and return in case of errors
        if(verbose){			// if we have -v parameter
        	sendLog("Failed to open directory");
        }
        closedir(dir);
        return;
    }

    while ((entry = readdir(dir)) != NULL) { // iterate by each file and folder from the starting directory
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) // skip current and parent directory
            continue;

        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry->d_name);


        if (verbose) {
			sprintf(message, "Checking file: %s, Pattern: %s", fullPath, keyword);
			sendLog(message);
        }
        

        
        if (lstat(fullPath, &statbuf) != 0) { // checks if entry cannot be accessed
        	if(verbose) {
				sprintf(message, "%s is not accessible", fullPath);
				sendLog(message);
        	}
            continue;		// skip to the next iteration
        }

        if (strstr(entry->d_name, keyword) != NULL) { // compare the entry name to searching file name
			sprintf(message, "Found: %s, Pattern: %s", fullPath, keyword);
			sendLog(message);
			*anyFilesFound = 1;
        }

        else if (S_ISDIR(statbuf.st_mode)) { // checks if entry is a directory
            search(fullPath, keyword, anyFilesFound, verbose);
        }
    }

    closedir(dir);
}