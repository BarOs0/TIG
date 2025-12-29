#ifndef MCAST_DISCOVER_H
#define MCAST_DISCOVER_H

/*Function that discovers the TIG server in the local network (TIG_cli)*/

#define MCAST_ADDR "ff02::2026"
#define MCAST_PORT 2026
#define DISCOVER_MSG "TIG_DISCOVERY"
#define RESPONSE_MSG "TIG_RESPONSE"
#define TIMEOUT 3
#define MSG_SIZE 64
#define MCAST_IF "wlp2s0"

int mcast_discover(void);

#endif // MCAST_DISCOVER