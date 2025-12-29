#ifndef DAEMON_H
#define DAEMON_H

#include <sys/types.h>

int daemon_init(const char *pname, int facility, uid_t uid);

#endif // DAEMON_H