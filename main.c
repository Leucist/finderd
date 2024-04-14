#include <stdio.h>
#include <string.h>
#include <stdlib.h>	// for atoi()
#include "search.h"

int main(int argc, char* argv[]) {
	// Handle params
	int delay = 30;	/* delay after search in seconds */
	int notificationsEnabled = 0;	/* whether daemon should send all notifications to the syslog (1) or not (0) */
	int keyWordsAmount = 0;		/* amount of the given keywords */
	char* keyWords[argc - 1];	/* array to store the keywords */

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-v")) notificationsEnabled = 1;
		else if (strcmp(argv[i], "-t")) {
			// Sets the delay value or report an error and stops the program
			if (++i >= argc || ((delay = atoi(argv[i])) == 0 && strcmp(argv[i], "0") != 0)) {
				fprintf(stderr, "Invalid delay input\n");
				return 1;
			}
		}
		else {
			keyWords[keyWordsAmount++] = argv[i];
		}
	}

	// Init the daemon

	while(1) {
		// process division

		// (child) search()
		// (parent) wait

		// (child) kills itself
		// (parent) kills children if any, sends report to syslog if no files have been found

		// sleep or wait
	}

	return 0;
} 
