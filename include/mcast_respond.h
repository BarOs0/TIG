#ifndef MCAST_RESPOND_H
#define MCAST_RESPOND_H

#define MCAST_ADDR "ff02::2026" // IPv6 multicast address
#define MCAST_PORT 2026 // Udp multicast port
#define DISCOVER_MSG "TIG_DISCOVERY" // Client discovery request
#define RESPONSE_MSG "TIG_RESPONSE" // Server discovery reponse
#define MSG_SIZE 32 // Discovery message buffer
#define MCAST_IF "br-50ca92fdda97" // Multicast interface (server)

void* mcast_respond(void* arg); // Server discovery response function - thread

#endif // MCAST_RESPOND