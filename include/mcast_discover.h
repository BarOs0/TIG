#ifndef MCAST_DISCOVER_H
#define MCAST_DISCOVER_H

#define MCAST_ADDR "ff02::2026" // IPv6 multicast address
#define MCAST_PORT 2026 // Udp multicast port
#define DISCOVER_MSG "TIG_DISCOVERY" // Client discovery request
#define RESPONSE_MSG "TIG_RESPONSE" // Server discovery reponse
#define TIMEOUT 3 // Seconds, socket timeout
#define MSG_SIZE 32 // Discovery message buffer
#define MCAST_IF "eth1" // Multicast interface (client, user configurable)

int mcast_discover(void); // Client discovery function

#endif // MCAST_DISCOVER