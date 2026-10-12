#include "engine/net/net.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
    typedef int net_socklen_t;
    #define NET_LASTERR() WSAGetLastError()
#else
    typedef socklen_t net_socklen_t;
    #define NET_LASTERR() errno
#endif

netaddr_t net_localaddr;
netsock_t net_sock;

#ifdef _WIN32
static bool InitWinsock( void )
{
    static bool initialized = false;
    WSADATA wsaData;

    if (initialized)
        return true;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return false;

    initialized = true;
    return true;
}
#endif

bool NET_Init( void )
{
#ifdef _WIN32
    if (!InitWinsock())
        return false;
#endif

    NET_GetLocalAddress();
    return true;
}

char* NET_AddrToString( netaddr_t addr )
{
    static char buff[64];   /* was a local array: returned a dangling pointer */
    struct in_addr ia;

    ia.s_addr = htonl(addr.ip);
    snprintf( buff, sizeof(buff), "%s:%u", inet_ntoa(ia), (unsigned)addr.port );

    return buff;
}

bool NET_StringToAddr( const char *s, unsigned short port, netaddr_t *a )
{
    struct addrinfo hints, *res = NULL;

    if (!s || !a)
        return false;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;

    /* inet_addr() can't resolve hostnames, and gethostname() returns one */
    if (getaddrinfo(s, NULL, &hints, &res) != 0 || !res)
        return false;

    a->ip   = ntohl(((struct sockaddr_in *)res->ai_addr)->sin_addr.s_addr);
    a->port = port;

    freeaddrinfo(res);
    return true;
}

void NET_GetLocalAddress( void )
{
    char buff[512];
    sockaddr_t address;
    net_socklen_t namelen;

    memset(&net_localaddr, 0, sizeof(net_localaddr));

    if (gethostname(buff, sizeof(buff)) != 0)
        buff[0] = 0;
    buff[sizeof(buff) - 1] = 0;

    if (!NET_StringToAddr(buff, 0, &net_localaddr))
        Con_printf("Could not resolve local hostname\n");
    else
        Con_printf("Local address: %s\n", NET_AddrToString(net_localaddr));

    /* No socket yet (NET_Init runs first), so there is no port to query. */
    if (!net_sock.open || net_sock.handle == SOCK_INVALID)
        return;

    namelen = sizeof(address);
    memset(&address, 0, sizeof(address));

    if (getsockname(net_sock.handle, ADRCST(address), &namelen) == -1)   /* was inverted */
        Sys_Error("getsockname: error %d\n", NET_LASTERR());

    net_localaddr.port = ntohs(address.sin_port);
    /* Keep the IP from the hostname lookup: a socket bound to 0.0.0.0 reports 0 here. */
    if (address.sin_addr.s_addr != htonl(INADDR_ANY))
        net_localaddr.ip = ntohl(address.sin_addr.s_addr);
}




void NET_SendData( const byte *data, int len, netsock_t sock, netaddr_t to )
{
    sockaddr_t sockaddr = netaddr_sockaddr(to);

    int res = sendto( 
            sock.handle, 
            data, 
            len, 
            0, 
            ADRCST(sockaddr), 
            sizeof(sockaddr) );
}

int NET_ReceiveData( byte *out, int len, netsock_t sock, netaddr_t *from )
{
    sockaddr_t sockaddr = netaddr_sockaddr(*from);
    socklen_t socklen = sizeof(sockaddr);

    int res = recvfrom( sock.handle, out, len,  0, &sockaddr, &socklen );
    if (res > 0 )
    {
       *from = sockaddr_netaddr( sockaddr ); 
    }
    return res;
}
