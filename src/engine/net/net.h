#ifndef NET_H
#define NET_H

#include <stdbool.h>

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

#include <common/common.h>

#ifdef _WIN32
typedef SOCKET netsockhandle_t;
#define SOCK_INVALID INVALID_SOCKET
#else
typedef int netsockhandle_t;
#define SOCK_INVALID (-1)
#endif

typedef struct sockaddr_in sockaddr_t;

typedef struct netsock_t
{
    netsockhandle_t handle;
    int open;
} netsock_t;

extern netsock_t net_sock;

// Host byte order
typedef struct netaddr_t
{
    unsigned int   ip;     
    unsigned short port;    
} netaddr_t;

typedef struct netchan_t
{
    netaddr_t remote;
    unsigned short qport;

    unsigned long incoming_sequence;
    unsigned long incoming_ack; // What we have last acknowledged

    unsigned long outgoing_sequence;
    unsigned long outgoing_ack; // What our remote last acknowledged

    unsigned long t_lastrecv;
    unsigned long t_lastsend;

} netchan_t;

typedef struct packet_t
{
    byte type;
    i32 length;
    byte *data;
} packet_t;

// Packet format [qport][length][sequence][type + data]
#define MSG_START (sizeof(short) + sizeof(long) * 2)

typedef struct challenge_t
{
    int challenge;
    float duration;
} challenge_t;

#define ADRCST(sockadr) ((struct sockaddr*)&(sockadr))

extern netaddr_t net_localaddr;
extern netsock_t net_sock;
extern netsock_t net_send_sock;

extern netaddr_t net_from;
extern byte net_message_buffer[MAX_NETMSG];

extern sizebuf_t net_message;

bool  NET_Init( void );
void  NET_GetLocalAddress( void );
char* NET_AddrToString( netaddr_t addr );
bool  NET_StringToAddr( const char *s, unsigned short port, netaddr_t *a );

FORCEINLINE bool NET_CompareAddresses( const netaddr_t a, const netaddr_t b )
{
    return (a.ip == b.ip) && (a.port == b.port);
}

void NET_SendData( const byte *data, int len, netsock_t sock, netaddr_t to );
int NET_ReceiveData( byte *out, int len, netsock_t sock, netaddr_t *from );

bool NET_GetPacket( void );


void Netchan_SendPacket( netchan_t *chan, netsock_t sock, int len, const byte *data );
bool Netchan_Process( netchan_t *chan );
void Netchan_Setup( netchan_t *chan);
void Netchan_ResetSequences( netchan_t *chan);


bool netsock_open( netsock_t* sock, unsigned short port );
void netsock_close( netsock_t* sock );

FORCEINLINE sockaddr_t netaddr_sockaddr( netaddr_t addr )
{
    sockaddr_t sadr = {0};
    sadr.sin_family = AF_INET;
    sadr.sin_port = htons(addr.port);
    sadr.sin_addr.s_addr = htonl(addr.ip);   /* s_addr works on Windows too */
    return sadr;
}

FORCEINLINE netaddr_t sockaddr_netaddr( sockaddr_t addr )
{
    netaddr_t nadr;
    nadr.ip   = ntohl( addr.sin_addr.s_addr );
    nadr.port = ntohs( addr.sin_port );
    return nadr;                              /* FIX: was missing, undefined behavior */
}

#endif
