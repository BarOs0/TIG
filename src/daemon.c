/**
 * @file daemon.c
 * @brief Daemon initialization implementation using double fork technique
 */

#include "daemon.h"
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

#define MAXFD 64

int daemon_init(const char *pname, int facility, uid_t uid){

    pid_t pid;

    if((pid = fork()) < 0){ ///< 1st fork()
        return -1;
    }
    else if(pid){
        exit(0); ///< Parent termination
    }

    /* Child 1 continues ... */

    if(setsid() < 0){ ///< Becoming session leader
        return -1;
    }

    signal(SIGHUP, SIG_IGN);
    if((pid = fork()) < 0){ ///< 2nd fork()
        return -1;
    }
    else if(pid){
        exit(0); ///< Child 1 termination
    }

    /* Child 2 continues ...*/

    chdir("/"); ///< Changing working directory

    for(int i = 0; i < MAXFD; i++){
        close(i);
    }

    /** Redirect stdin, stdout, stderr to /dev/null black hole */ 
    
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_RDWR);
    open("/dev/null", O_RDWR);

    openlog(pname, LOG_PID, facility);

    setuid(uid); ///< Changing user

    return 0;
}