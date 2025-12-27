#ifndef MCAST_DISCOVER_H
#define MCAST_DISCOVER_H

#define MCAST_ADDR "ff02::2026"
#define MCAST_PORT 2026
#define DISCOVER_MSG "TIG_DISCOVERY"
#define RESPONSE_MSG "TIG_RESPONSE"
#define TIMEOUT 2 // seconds
#define BUFF_SIZE 64

void mcast_discover(void);

#endif // MCAST_DISCOVER