#ifndef MCAST_RESPOND_H
#define MCAST_RESPOND_H

#define MCAST_ADDR "ff02::2026"
#define MCAST_PORT 2026
#define DISCOVER_MSG "TIG_DISCOVERY"
#define RESPONSE_MSG "TIG_RESPONSE"
#define BUFF_SIZE 64

void mcast_respond(void);

#endif // MCAST_RESPOND