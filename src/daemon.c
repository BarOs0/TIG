#include "daemon.h"
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>

#define MAXFD 64

int daemon_init(const char *pname, int facility, uid_t uid){

    pid_t pid;

    if((pid = fork()) < 0){ // 1st fork()
        return -1;
    }
    else if(pid){
        exit(0); // parent termination
    }

    /* child 1 continues ... */

    if(setsid() < 0){ // becoming session leader
        return -1;
    }

    signal(SIGHUP, SIG_IGN);
    if((pid = fork()) < 0){ // 2nd fork()
        return -1;
    }
    else if(pid){
        exit(0); // child 1 termination
    }

    /* child 2 continues ...*/

    chdir("/"); // changing working directory

    for(int i = 0; i < MAXFD; i++){
        close(i);
    }

    /* redirect stdin, stdout, stderr to /dev/null black hole */ 
    
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_RDWR);
    open("/dev/null", O_RDWR);

    openlog(pname, LOG_PID, facility);

    setuid(uid); // changing user

    return 0;
}