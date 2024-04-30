#include <syslog.h>

#include "sendLog.h"
#include "setCurrentTime.h"

void sendLog(char *message) {
	// opens connection with the syslog (user-level messages with pid in each one)
	openlog("finderd", LOG_PID, LOG_USER);
	// sets the current time
	char time_str[64];
	setCurrentTime(time_str, sizeof(time_str));
	// send the message to the syslog daemon
	syslog(LOG_INFO, "%s | %s", time_str, message);

	// closes the open syslog connection
	closelog();
}