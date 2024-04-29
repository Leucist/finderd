#include <stdio.h>
#include <string.h>
#include <stdlib.h>		// for atoi()
#include <sys/types.h>	// for pid_t
#include <unistd.h>		// for fork(), setsid(), sleep(), sysconf()
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




	if (signo == SIGUSR1 || signo == SIGUSR2 || signo == SIGINT) {
		// if (childrenAmount > 0) {
			// redirects the signal to the children
			killChildren(signo);
		// }

		if (signo == SIGINT) {	/* < to avoid creating orphans in any way */
			free(children);
			closelog();
			exit(-1);
		}
	}
}

// void handleChildSignals(int signo) {
// 	// sends signal SIGUSR1 to the parent process and terminate
// 	kill(getppid(), signo);
// 	exit(0);
// }

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
	// if(daemonise()) return -1;
	if(daemon(0, 0)) return -1;

	
	// opens connection with the syslog (user-level messages with pid in each one)
	openlog("finderd", LOG_PID, LOG_USER);

	while(1) {


		syslog(LOG_INFO, "- - - - - - - - - - - - - - - - - - - - - - -");


		// notifies about daemon awakening
		if (notificationsEnabled) {
			setCurrentTime(time_str, sizeof(time_str));
			sprintf(message, "%s | Daemon woke up", time_str);
			syslog(LOG_INFO, "%s", message);
		}



		// - Setting signal mask to ignore all signals except SIGUSR1, SIGUSR2
		// sigset_t maskSet;
		// sigfillset(&maskSet);
		// sigdelset(&maskSet, SIGUSR1);
		// sigdelset(&maskSet, SIGUSR2);
		// sigprocmask(SIG_SETMASK, &maskSet, NULL);

		// setCurrentTime(time_str, sizeof(time_str));
		// sprintf(message, "%s | Signal mask set", time_str);
		// syslog(LOG_INFO, "%s", message);




		// - Process Division
		pid_t pid;

		childrenAmount = keyWordsAmount;	/* resets the counter */
		children = (pid_t *)malloc(childrenAmount * sizeof(pid_t));

		syslog(LOG_INFO, "Amount of children(keywords): %d", childrenAmount);

		for (int i = 0; i < childrenAmount; i++) {
			pid = fork();
			if (pid < 0) return 1;
			else if (pid == 0) {			// < for the child process


				// setCurrentTime(time_str, sizeof(time_str));
				// sprintf(message, "%s | [CHILD] Child process %d started", time_str, getpid());
				// syslog(LOG_INFO, "%s", message);


				// // - Setting signal mask to ignore all signals except SIGUSR1, SIGUSR2
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

				// struct sigaction ch_sa;
				// ch_sa.sa_handler = handleChildSignals;
				// sigemptyset(&ch_sa.sa_mask);
				// ch_sa.sa_flags = 0;

				int anyFilesFound = 0;
				search(".", keyWords[i], &anyFilesFound, notificationsEnabled);
				// sleep(5);


				// setCurrentTime(time_str, sizeof(time_str));
				// sprintf(message, "%s | [CHILD] Child process %d exits now", time_str, getpid());
				// syslog(LOG_INFO, "%s", message);


				exit(anyFilesFound);
			}
			children[i] = pid;
		}
		
		// Parent process continues >
		signal(SIGUSR1, handleSignals);
		signal(SIGUSR2, handleSignals);
		// signal(SIGCHLD, handleSignals);
		signal(SIGINT, handleSignals);



		// setCurrentTime(time_str, sizeof(time_str));
		// sprintf(message, "%s | Parent enabled signal handling for SIGUSR1 and SIGUSR2", time_str);
		// syslog(LOG_INFO, "%s", message);



		// - Infinite loop that waits till the child processes terminate
		int childrenExitStatuses[childrenAmount];
		int termSig = 0;	/* store the signo that terminated children */
		// int childrenTerminated = 0;

		// while (childrenTerminated < childrenAmount) {
		// 	childrenTerminated = 0;	/* resets the counter */

		// 	// constantly checks if any child was terminated
		// 	for (int childno = 0; childno < childrenAmount; childno++) {
		// 		// gets child' exit status in the childExitStatus variable
		// 		waitpid(children[childno], &childrenExitStatuses[childno], WNOHANG);

		// 		// - Analysing the gathered status
		// 		// if child exited normally
		// 		if (WIFEXITED(childrenExitStatuses[childno])) {
		// 			childrenTerminated++;

		// 			setCurrentTime(time_str, sizeof(time_str));
		// 			sprintf(message, "%s | [PARENT]: Child process %d exited normally. Children terminated: %d", time_str, children[childno], childrenTerminated);
		// 			syslog(LOG_INFO, "%s", message);
		// 		}

		// 		// if child was terminated by the signal
		// 		else if (WIFSIGNALED(childrenExitStatuses[childno])) {
		// 			// gathers the signal that killed the child
		// 			termSig = WTERMSIG(childrenExitStatuses[childno]);

		// 			setCurrentTime(time_str, sizeof(time_str));
		// 			sprintf(message, "%s | [PARENT]: Child process %d terminated by the signal %d: %s", time_str, children[childno], termSig, strsignal(termSig));
		// 			syslog(LOG_INFO, "%s", message);



		// 			// terminates all the other children
		// 			killChildren(SIGKILL);	/* send SIGKILL to all the children */
		// 			childrenTerminated = childrenAmount;	/* < to end the loop */
		// 			break;
		// 		}
		// 	}
		// }

		/* contruction that is waiting till the children end one by one in the fixed order, simultaneously checking exit statuses */
		for (int childno = 0; childno < childrenAmount; childno++) {
			waitpid(children[childno], &childrenExitStatuses[childno], 0);
			if (WIFSIGNALED(childrenExitStatuses[childno])) {
				termSig = WTERMSIG(childrenExitStatuses[childno]);
				syslog(LOG_INFO, "TERMSIG = %s", strsignal(termSig));
			}
		}

		// - Freeing the memory allocated with the malloc()
		free(children);



		setCurrentTime(time_str, sizeof(time_str));
		sprintf(message, "%s | Children were freed", time_str);
		syslog(LOG_INFO, "%s", message);




		// - Daemon handles the termSig
		// (gathers the exit codes of the children which indicate whether any files were found if children exited normally)
		int filesFound = 0;
		// if children exited normally (termSig hasn't changed)
		if (termSig == 0) {
			// adds up to the filesFound if any files were found
			for (int childno = 0; childno < childrenAmount; childno++) {
				filesFound += WEXITSTATUS(childrenExitStatuses[childno]);
			}
		}
		// if child process was terminated by the signal
		else {
			if (notificationsEnabled) {
				setCurrentTime(time_str, sizeof(time_str));
				sprintf(message, "%s | Daemon was interrupted by the signal number %d: %s", time_str, termSig, strsignal(termSig));
				syslog(LOG_INFO, "%s", message);
			}
			if (termSig == SIGUSR1) { /* reruns the search immediately */
				closelog();
				continue;	// skips the sleep and restarts immediately
			}
		}

		// - Notifying if no files were found
		if (filesFound <= 0) {
			setCurrentTime(time_str, sizeof(time_str));
			sprintf(message, "%s | No files found", time_str);
			syslog(LOG_INFO, "%s", message);
		}

		// - Notifying about putting the daemon to sleep
		if (notificationsEnabled) {
			setCurrentTime(time_str, sizeof(time_str));
			sprintf(message, "%s | Daemon fell asleep", time_str);
			syslog(LOG_INFO, "%s", message);
		}

		childrenAmount = 0;		/* clears the counter */
		// sets SIGUSR2 handling as 'ignore' so it wouldn't awaken the daemon
		signal(SIGUSR2, SIG_IGN);
		// closes the open syslog connection
		closelog();
		// - Putting the daemon to sleep
		sleep(delay);
	}
	return 0;
} 
