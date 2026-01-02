#ifndef DAEMON_H
#define DAEMON_H

/**
 * @file daemon.h
 * @brief Daemon process initialization function
 */

#include <sys/types.h>

/**
 * @brief Daemon process initialization function
 * @param pname Process name
 * @param facility Syslog facility
 * @param uid User ID
 * @return 0 on success, -1 on error
 */
int daemon_init(const char *pname, int facility, uid_t uid);

#endif // DAEMON_H