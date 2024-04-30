#ifndef SEND_LOG_H
#define SEND_LOG_H


/**
 * Sends given string to the SYSLOG appending current timestamp at the beginning.
 *
 * Parameters:
 *    message: String to be sent to the SYSLOG.
 *
 * Returns:
 *    None.
 */

void sendLog(char *message);

#endif