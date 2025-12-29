#ifndef MCAST_RESPOND_H
#define MCAST_RESPOND_H

/*Function responding to discover message from TIG server in the local network (TIG_srv)*/

#define MCAST_ADDR "ff02::2026"
#define MCAST_PORT 2026
#define DISCOVER_MSG "TIG_DISCOVERY"
#define RESPONSE_MSG "TIG_RESPONSE"
#define MSG_SIZE 64
#define MCAST_IF "wlp2s0"

void* mcast_respond(void* arg);

#endif // MCAST_RESPOND