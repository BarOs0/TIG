#ifndef MCAST_RESPOND_H
#define MCAST_RESPOND_H

#define MCAST_ADDR "ff02::2026"
#define MCAST_PORT 2026
#define DISCOVER_MSG "TIG_DISCOVERY"
#define RESPONSE_MSG "TIG_RESPONSE"
#define MSG_SIZE 64
#define MCAST_IF "br-50ca92fdda97"

void* mcast_respond(void* arg);

#endif // MCAST_RESPOND