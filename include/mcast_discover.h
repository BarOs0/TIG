#ifndef MCAST_DISCOVER_H
#define MCAST_DISCOVER_H

/**
 * @file mcast_discover.h
 * @brief Client multicast discovery function for server detection
 */

#define MCAST_ADDR "ff02::2026" ///< IPv6 multicast address
#define MCAST_PORT 2026 ///< UDP multicast port
#define DISCOVER_MSG "TIG_DISCOVERY" ///< Client discovery request
#define RESPONSE_MSG "TIG_RESPONSE" ///< Server discovery response
#define TIMEOUT 3 ///< Seconds, socket timeout
#define ADDR_BUFF_SIZE 1024 ///< Server addresses buffer
#define MCAST_IF "eth0" ///< Multicast interface (client, user configurable)

/**
 * @brief Client discovery function
 * @return 0 on success, -1 on error
 */
int mcast_discover(void);

#endif // MCAST_DISCOVER