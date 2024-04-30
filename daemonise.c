#include "daemonise.h"

int daemonise() {
	// constant to define the max_fd if sysconf(_SC_OPEN_MAX) fails
	const int MAX_FD = 1024;

	// fork reassures that the process is not the current group leader
	switch(fork()) {
		case -1: return -1;
		case 0: break;                  // child falls through
		default: exit(EXIT_SUCCESS);	// parent terminates
	}

	if (setsid() == -1) {				// makes process the leader of the newly created session
		perror("Error occured in setsid()"); 
		return -1;
	}

	// fork reassures that the process is not the group leader
	switch(fork()) {
		case -1: return -1;
		case 0: break;                  // child falls through
		default: exit(EXIT_SUCCESS);	// parent terminates
	}

	// changes the current directory to the root directory
	if (chdir("/") == -1) {
		perror("Error occured in chdir()"); 
		return -1;
	}

	// closes all file descriptors
	int max_fd = sysconf(_SC_OPEN_MAX);
	if (max_fd == -1) max_fd = MAX_FD;
	for (int fd = 0; fd < max_fd; fd++)
		close(fd);

	// closes stdin then points stdout and stderr to '/dev/null'
	close(STDIN_FILENO);
	int fd = open("/dev/null", O_RDWR);
	if(fd != STDIN_FILENO)
		return -1;
	if(dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
		return -2;
	if(dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
		return -3;

	return 0;
} 
