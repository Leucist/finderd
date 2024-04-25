#include <stdio.h>
#include <string.h>
#include <stdlib.h>		// for atoi()
#include <sys/types.h>	// for pid_t
#include <unistd.h>		// for fork(), setsid(), sleep()
#include <fcntl.h>		// for handling file descriptors
#include <syslog.h>
#include <signal.h>
#include <sys/wait.h>	// for waitpid()

#include "search.h"
#include "setCurrentTime.h"

pid_t *children;
int childrenAmount = 0;

void killChildren(int signo) {
	for (int i = 0; i < childrenAmount; i++) {
		kill(children[i], signo);
	}
}

void handleSignals(int signo) {



	char time_str[64];
    char message[256];
	setCurrentTime(time_str, sizeof(time_str));
	sprintf(message, "%s | [PARENT]: Received signal %d: %s", time_str, signo, strsignal(signo));
	syslog(LOG_INFO, "%s", message);




	if (signo == SIGUSR1 || signo == SIGUSR2) {
		killChildren(signo);
	}
}

int deamonise() {
	// constant to define the max_fd if sysconf(_SC_OPEN_MAX) fails
	const int MAX_FD = 8192;

	// fork reassures that the process is not the current group leader
	switch(fork()) {
		case -1: return -1;
		case 0: break;                  // child falls through
		default: _exit(EXIT_SUCCESS);   // parent terminates
	}

	if (setsid() == -1) {				// makes process the leader of the newly created session
		perror("Error occured in setsid()"); 
		return -1;
	}

	// fork reassures that the process is not the group leader
	switch(fork()) {
		case -1: return -1;
		case 0: break;                  // child falls through
		default: _exit(EXIT_SUCCESS);   // parent terminates
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

int main(int argc, char* argv[]) {
	// Handle params
	int delay = 30;	/* delay after search in seconds */
	int notificationsEnabled = 0;	/* whether daemon should send all notifications to the syslog (1) or not (0) */
	int keyWordsAmount = 0;		/* amount of the given keywords */
	char* keyWords[argc - 1];	/* array to store the keywords */

    char time_str[64];
    char message[256];

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-v") == 0) notificationsEnabled = 1;
		else if (strcmp(argv[i], "-t") == 0) {
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
	if(deamonise()) return -1;

	
	// opens connection with the syslog (user-level messages with pid in each one)
	openlog("finderd", LOG_PID, LOG_USER);






	
	// ----------------------
	// ----------------------
	// pid_t pid;
	// for (int i = 0; i < keyWordsAmount; i++) {
	// 	pid = fork();
	// 	if (pid < 0) return 1;
	// 	else if (pid == 0) {			// < for the child process
	// 		int anyFilesFound = 0;
	// 		search(".", keyWords[i], &anyFilesFound, notificationsEnabled);
	// 		exit(anyFilesFound);
	// 	}
	// }
	// // Parent process continues >
	// signal(SIGUSR1, handleSignals);
	// signal(SIGUSR2, handleSignals);

	// int childExitStatuses[childrenAmount];
	// int childrenTerminated = 0;

	// while (childrenTerminated < childrenAmount) {
	// 	childrenTerminated = 0;	/* resets the counter */
	// 	// constantly checks if any child was terminated
	// 	for (int childno = 0; childno < childrenAmount; childno++) {
	// 		waitpid(children[childno], &childExitStatuses[childno], WNOHANG);
	// 		// if child was terminated by the signal
	// 		if (WIFSIGNALED(childExitStatuses[childno])) {



	// 			int termSig = WTERMSIG(childExitStatuses[childno]);
	// 			setCurrentTime(time_str, sizeof(time_str));
	// 			sprintf(message, "%s | [PARENT]: Child process %d terminated by the signal %d: %s", time_str, children[childno], termSig, strsignal(termSig));
	// 			syslog(LOG_INFO, "%s", message);




	// 			killChildren(SIGKILL);	/* send SIGKILL to all the children */
	// 			childrenTerminated = childrenAmount;	/* < to end the loop */
	// 			break;
	// 		}
	// 		// if child exited normally
	// 		else if (WIFEXITED(childExitStatuses[childno])) {
	// 			childrenTerminated++;


	// 			setCurrentTime(time_str, sizeof(time_str));
	// 			sprintf(message, "%s | [PARENT]: Child process %d exited normally. Children terminated: %d", time_str, children[childno], childrenTerminated);
	// 			syslog(LOG_INFO, "%s", message);
	// 		}
	// 	}
	// }
	// exit(100);
	// ----------------------
	// ----------------------






	while(1) {
		// notifies about daemon awakening
		if (notificationsEnabled) {
			setCurrentTime(time_str, sizeof(time_str));
			sprintf(message, "%s | Daemon woke up", time_str);
			syslog(LOG_INFO, "%s", message);
		}


		// - Process Division
		pid_t pid;

		childrenAmount = keyWordsAmount;
		children = (pid_t *)malloc(childrenAmount * sizeof(pid_t));

		for (int i = 0; i < keyWordsAmount; i++) {
			pid = fork();
			if (pid < 0) return 1;
			else if (pid == 0) {			// < for the child process


				setCurrentTime(time_str, sizeof(time_str));
				sprintf(message, "%s | [CHILD] Child process %d started", time_str, getpid());
				syslog(LOG_INFO, "%s", message);


				// - Setting signal mask to ignore all signals except SIGUSR1, SIGUSR2
				// sigset_t maskSet;
				// sigfillset(&maskSet);
				// sigdelset(&maskSet, SIGUSR1);
				// sigdelset(&maskSet, SIGUSR2);
				// sigprocmask(SIG_SETMASK, &maskSet, NULL);

				// setCurrentTime(time_str, sizeof(time_str));
				// sprintf(message, "%s | [CHILD %d] Signal mask set", time_str, getpid());
				// syslog(LOG_INFO, "%s", message);



				// - Setting SIGUSR1 and SIGUSR2 handling as default
				signal(SIGUSR1, SIG_DFL);
				signal(SIGUSR2, SIG_DFL);

				int anyFilesFound = 0;
				search(".", keyWords[i], &anyFilesFound, notificationsEnabled);
				// sleep(1);


				setCurrentTime(time_str, sizeof(time_str));
				sprintf(message, "%s | [CHILD] Child process %d ends now", time_str, getpid());
				syslog(LOG_INFO, "%s", message);


				exit(anyFilesFound);
			}
			children[i] = pid;
		}
		
		// Parent process continues >
		signal(SIGUSR1, handleSignals);
		signal(SIGUSR2, handleSignals);
		// signal(SIGCHLD, handleSignals);



		setCurrentTime(time_str, sizeof(time_str));
		sprintf(message, "%s | Parent enabled signal handling for SIGUSR1 and SIGUSR2", time_str);
		syslog(LOG_INFO, "%s", message);



		// - Infinite loop that waits till the child processes terminate
		int childExitStatuses[childrenAmount];
		int childrenTerminated = 0;

		while (childrenTerminated < childrenAmount) {
			childrenTerminated = 0;	/* resets the counter */
			// constantly checks if any child was terminated
			for (int childno = 0; childno < childrenAmount; childno++) {
				waitpid(children[childno], &childExitStatuses[childno], WNOHANG);
				// if child was terminated by the signal
				if (WIFSIGNALED(childExitStatuses[childno])) {



					int termSig = WTERMSIG(childExitStatuses[childno]);
					setCurrentTime(time_str, sizeof(time_str));
					sprintf(message, "%s | [PARENT]: Child process %d terminated by the signal %d: %s", time_str, children[childno], termSig, strsignal(termSig));
					syslog(LOG_INFO, "%s", message);




					killChildren(SIGKILL);	/* send SIGKILL to all the children */
					childrenTerminated = childrenAmount;	/* < to end the loop */
					break;
				}
				// if child exited normally
				else if (WIFEXITED(childExitStatuses[childno])) {
					childrenTerminated++;


					setCurrentTime(time_str, sizeof(time_str));
					sprintf(message, "%s | [PARENT]: Child process %d exited normally. Children terminated: %d", time_str, children[childno], childrenTerminated);
					syslog(LOG_INFO, "%s", message);
				}
			}
		}

		/* obsolete contruction that was waiting till the children end one by one in the fixed order */
		// for (int childno = 0; childno < childrenAmount; childno++) {
		// 	waitpid(children[childno], &childExitStatuses[childno]);
		// }

		// - Freeing the memory allocated with the malloc()
		free(children);



		setCurrentTime(time_str, sizeof(time_str));
		sprintf(message, "%s | Children were freed", time_str);
		syslog(LOG_INFO, "%s", message);




		// - Daemon goes through the terminated children once again and collects data
		int filesFound = 0;
		// if child process was terminated by the signal
		if (WIFSIGNALED(childExitStatuses[0])) {
			int termSig = WTERMSIG(childExitStatuses[0]);	// gets the signal that terminated the child process
			if (notificationsEnabled) {
				setCurrentTime(time_str, sizeof(time_str));
				sprintf(message, "%s | Daemon was interrupted by the signal number %d: %s", time_str, termSig, strsignal(termSig));
				syslog(LOG_INFO, "%s", message);
			}
			if (termSig == SIGUSR1) {
				continue;	// skips the sleep and restarts immidiately
			}
		}
		// if children exited normally
		else if (WIFEXITED(childExitStatuses[0])) {
			// adds up to the filesFound if any files were found
			for (int childno = 0; childno < childrenAmount; childno++) {
				filesFound += WEXITSTATUS(childExitStatuses[childno]);
			}
		}
		// if no files found
		else if (filesFound <= 0) {
			setCurrentTime(time_str, sizeof(time_str));
			sprintf(message, "%s | No files found", time_str);
			syslog(LOG_INFO, "%s", message);
		}

		// notifies about putting the daemon to sleep
		if (notificationsEnabled) {
			setCurrentTime(time_str, sizeof(time_str));
			sprintf(message, "%s | Daemon fell asleep", time_str);
			syslog(LOG_INFO, "%s", message);
		}

		// - Putting the daemon to sleep
		sleep(delay);
	}

	closelog();

	return 0;
} 
