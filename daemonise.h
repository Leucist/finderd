#ifndef DAEMONISE_H
#define DAEMONISE_H

/**
 * Makes calling proccess a daemon.
 *
 * Parameters:
 *    None.
 *
 * Returns:
 *    0  - on success.
 *    -1 - error occured.
 *    -2 - error in redirecting STDOUT.
 *    -3 - error in redirecting STDERR.
 */

int daemonise();

#endif 
