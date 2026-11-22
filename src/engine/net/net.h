#ifndef __NET_H__
#define __NET_H__

#ifdef _WIN32
    /* FIX: Define WIN32_LEAN_AND_MEAN to exclude unneeded Windows headers (like Fibers) */
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <arpa/inet.h>
    #include <netinet/in.h>   /* sockaddr_in */
    #include <netdb.h>        /* getaddrinfo */
    #include <sys/socket.h>
    #include <unistd.h>
#endif

#include "core/defs.h"



typedef struct netaddr_t
{
    u32 ip;
    u16 port;

    FORCEINLINE bool operator== (const netaddr_t& other)
    {
        return (other.ip == ip) && (other.port == port);
    }
    FORCEINLINE bool operator!= (const netaddr_t& other)
    {
        return !((*this)==other);
    }

    bool FromString (const char *address) const;

    void ToString (char* out, int n);

} netaddr_t;

extern netaddr_t net_local_addr;

#endif