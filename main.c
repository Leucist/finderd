#include <stdio.h>
#include <string.h>
#include <stdlib.h>		// for atoi()
#include <sys/types.h>	// for pid_t
#include <unistd.h>		// for fork(), setsid(), sleep(), sysconf()
#include <fcntl.h>		// for handling file descriptors
#include <syslog.h>
#include <signal.h>
#include <sys/wait.h>	// for waitpid()

#include "daemonise.h"
#include "sendLog.h"
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
	// sends the notification about caught signal to the SYSLOG
    char message[256];
	sprintf(message, "[PARENT]: Received signal %d: %s", signo, strsignal(signo));
	sendLog(message);

	if (signo == SIGUSR1 || signo == SIGUSR2 || signo == SIGINT) {
		// redirects the signal to the children
		killChildren(signo);

		if (signo == SIGINT) {	/* < to avoid creating orphans in any way */
			free(children);
			exit(-1);
		}
	}
}

int main(int argc, char* argv[]) {
	// Handle params
	int delay = 30;	/* delay after search in seconds */
	int notificationsEnabled = 0;	/* whether daemon should send all notifications to the syslog (1) or not (0) */
	int keyWordsAmount = 0;		/* amount of the given keywords */
	char* keyWords[argc - 1];	/* array to store the keywords */

    char message[256];			/* string to contain message to the SYSLOG */

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
	if(daemonise()) return -1;
	// if(daemon(0, 0)) return -1;

	while(1) {
		// <- just to separate daemon log entries visually when there are many
		sendLog("- - - - - - - - - - - - - - - - - - - - - - -");
		// notifies about daemon awakening
		if (notificationsEnabled) {
			sendLog("Daemon woke up");
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

		if (notificationsEnabled) {
			sprintf(message, "Amount of children(keywords): %d", childrenAmount);
			sendLog(message);
		}

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
			}
		}

		// - Freeing the memory allocated with the malloc()
		free(children);

		if (notificationsEnabled)
			sendLog("Children were freed");

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
				sprintf(message, "Daemon was interrupted by the signal number %d: %s", termSig, strsignal(termSig));
				sendLog(message);
			}
			if (termSig == SIGUSR1) { /* reruns the search immediately */
				continue;	// skips the sleep
			}
		}

		// - Notifying if no files were found
		if (filesFound <= 0) {
			sendLog("No files found");
		}

		// - Notifying about putting the daemon to sleep
		if (notificationsEnabled) {
			sendLog("Daemon fell asleep");
		}

		childrenAmount = 0;		/* clears the counter */
		// sets SIGUSR2 handling as 'ignore' so it wouldn't awaken the daemon
		signal(SIGUSR2, SIG_IGN);
		// - Putting the daemon to sleep
		sleep(delay);
	}
	return 0;
} 
