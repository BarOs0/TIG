#ifndef DAEMON_H
#define DAEMON_H

/*Daemon process initialization function*/

#include <sys/types.h>

int daemon_init(const char *pname, int facility, uid_t uid);

#endif // DAEMON_H