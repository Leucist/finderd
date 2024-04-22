#include <stdio.h>
#include <string.h>
#include <stdlib.h>		// for atoi()
#include <sys/types.h>	// for pid_t
#include <unistd.h>		// for fork(), setsid(), sleep()
#include <fcntl.h>		// for handling file descriptors
#include <syslog.h>
#include <time.h>
#include <signal.h>

#include "search.h"
#include "setCurrentTime.h"

pid_t *children;
int childrenAmount = 0;

void handleSignals(int signo) {
	if (signo == SIGUSR1 || signo == SIGUSR2) {
		for (int i = 0; i < childrenAmount; i++) {
			// terminates children
			kill(children[i], signo);
		}
	}
}

int main(int argc, char* argv[]) {
	// Handle params
	int delay = 30;	/* delay after search in seconds */
	int notificationsEnabled = 0;	/* whether daemon should send all notifications to the syslog (1) or not (0) */
	int keyWordsAmount = 0;		/* amount of the given keywords */
	char* keyWords[argc - 1];	/* array to store the keywords */

	time_t current_time;
    struct tm *local_time;
    char time_str[64];
    char message[256];

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
	pid_t daemon_pid;
	daemon_pid = fork();							// creates new process
	if (daemon_pid == -1) {
		perror("Error occured in fork()"); 
		return 1;
	}
	else if (daemon_pid != 0) exit(EXIT_SUCCESS);	// terminates the parent process

	if (setsid() == -1) {
		perror("Error occured in setsid()"); 
		return 1;
	}

	if (chdir("/") == -1) {
		perror("Error occured in chdir()"); 
		return 1;
	}

	// closes all file descriptors
	int max_fd = sysconf(_SC_OPEN_MAX);
	for (int fd = 0; fd < max_fd; fd++) close(fd);

	// redirects fd's 0, 1, 2 to '/dev/null'
	open("/dev/null", O_RDWR);	/* stdin */
	dup(0);						/* stdout */
	dup(0);						/* stderror */

	// opens connection with the syslog (user-level messages with pid in each one)
	openlog("finderd", LOG_PID, LOG_USER);

	while(1) {
		// notification
		if (notificationsEnabled) {
			setCurrentTime(&current_time, local_time, time_str, sizeof(time_str));
			sprintf(message, "%s | Daemon woke up", time_str);
			syslog(LOG_INFO, "%s", message);
		}

		// process division
		pid_t pid;

		childrenAmount = keyWordsAmount;
		children = (pid_t *)malloc(childrenAmount * sizeof(pid_t));

		for (int i = 0; i < keyWordsAmount; i++) {
			pid = fork();
			if (pid < 0) return 1;
			else if (pid == 0) break;	// throws child out of the loop
			children[i] = pid;
		}
		if (pid == 0) {					// for the child process
			int anyFilesFound;
			anyFilesFound = search(".", keyWords[i]) > 0 ? 1 : 0;
			exit(anyFilesFound);
		}
		// Parent process continues >
		signal(SIGUSR1, handleSignals);
		signal(SIGUSR2, handleSignals);
		// signal(SIGCHLD, handleSignals);

		int childExitStatuses[childrenAmount];
		for (int childno = 0; childno < childrenAmount; childno++) {
			waitpid(children[childno], &childExitStatuses[childno]);
		}

		// frees the memory after malloc()
		free(children);

		int filesFound = 0;
		// if child process was terminated by the signal
		if (WIFSIGNALED(childExitStatuses[0])) {
			int termSig = WTERMSIG(childExitStatuses[0]);	// gets the signal that terminated the child process
			if (notificationsEnabled) {
				setCurrentTime(&current_time, local_time, time_str, sizeof(time_str));
				sprintf(message, "%s | Daemon was interrupted via the %s signal", time_str, strsignal(termSig));
				syslog(LOG_INFO, "%s", message);
			}
			if (termSig == SIGUSR1) {
				continue;
			}
		}
		// if children exited normally
		else if (WIFEXITED(childExitStatuses[0])) {
			// adds the filesFound
			for (int childno = 0; childno < childrenAmount; childno++) {
				filesFound += WEXITSTATUS(childExitStatuses[childno]);
			}
		}
		// if no files found
		else if (filesFound <= 0) {
			setCurrentTime(&current_time, local_time, time_str, sizeof(time_str));
			sprintf(message, "%s | No files found", time_str);
			syslog(LOG_INFO, "%s", message);
		}

		// notifies about the daemon falling asleep
		if (notificationsEnabled) {
			setCurrentTime(&current_time, local_time, time_str, sizeof(time_str));
			sprintf(message, "%s | Daemon fell asleep", time_str);
			syslog(LOG_INFO, "%s", message);
		}

		// daemon sleeps
		sleep(delay);
	}

	closelog();

	return 0;
} 
