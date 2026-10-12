#include "engine/net/net.h"
#include "engine/system/message.h"


#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <unistd.h>
#endif




bool netsock_open( netsock_t* sock, unsigned short port )
{
    if (!sock) return false;

    sock->handle = socket(AF_INET, SOCK_DGRAM, 0);
    #ifdef _WIN32
        sock->open = sock->handle != SOCK_INVALID;
    #else
        sock->open = sock->handle >= 0;
    #endif
    if (!sock->open) return false;

    netaddr_t nadr;
    nadr.ip = 0; nadr.port = port;
    sockaddr_t sockaddr = netaddr_sockaddr( nadr );

    int res = bind( 
        sock->handle, 
        ADRCST( sockaddr ),
        sizeof( sockaddr )
    );
    if ( res < 0 ){
#ifdef _WIN32
        //printf("bind() failed: %d\n", WSAGetLastError());
#else
        //perror("bind");
#endif
        netsock_close( sock );
        return false;
    }

#ifdef _WIN32
    // Make the socket non-blocking.
    u_long nonblocking = 1;

    if (ioctlsocket( sock->handle, FIONBIO, &nonblocking ) != 0) {
        //printf("ioctlsocket() failed: %d\n", WSAGetLastError());
        netsock_close( sock );
        return false;
    }
#endif

    return true;
}

void netsock_close( netsock_t* sock )
{
#ifdef _WIN32
    if (sock->open){
        closesocket( sock->handle );
        sock->handle = SOCK_INVALID;
    }
#else
    if (sock->open){
        close( sock->handle );
        sock->handle = SOCK_INVALID;
    }
#endif
}