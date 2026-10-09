#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <arpa/inet.h>
    #include <sys/socket.h>
#endif
#include "common/common.h"

using byte = unsigned char;
using netres_t = ssize_t;
#define NET_MAX_PACKET 512

#define NET_SERVER_DEFAULT_PORT 27015
#define NET_CLIENT_DEFAULT_PORT 27005

#ifdef _WIN32
typedef SOCKET netsockhandle_t;
#else
typedef int netsockhandle_t;
#endif


struct NetAddress
{
    uint32_t ip;
    uint16_t port;

    bool operator==(const NetAddress& other) const {return (ip == other.ip) && (port == other.port);};
    bool operator!=(const NetAddress& other) const { return !(*this == other); };

    FORCEINLINE struct sockaddr_in SockAddr( void );
    inline const char* ToString( char* out, size_t outlen );
    inline NetAddress FromString( const char* ip, u16 port );
};

FORCEINLINE struct sockaddr_in NetAddress::SockAddr( void ){
    struct sockaddr_in adr;
    adr.sin_family = AF_INET;
    adr.sin_port = htons(port);
    adr.sin_addr.s_addr = htonl(ip);
    return adr;
}

inline const char* NetAddress::ToString( char* out, size_t outlen ){
    struct in_addr ia;
    ia.s_addr = htonl(ip);
    snprintf(out, outlen, "%s:%u", inet_ntoa(ia), port);
    return out;
}


inline NetAddress NetAddress::FromString( const char* ip, u16 port){
    NetAddress adr;
    adr.port = port;
    adr.ip = ntohl(inet_addr(ip));
    return adr;
}


struct challenge_t
{
    NetAddress dest;
    u32 challenge;
    float duration;
};
#define NETCHALLENGE_SIZE ( sizeof(((challenge_t*)(0))->challenge))

enum class PacketType : u8
{   
    
    ChallengeReq = 0,
    ChallengeSend = 1,
    ChallengeApprove = 2,
    ChallengeDenied = 3,
};

enum class HeaderType : u8
{
    Connectionless,
    Connected,
};

struct NetPacket
{
    NetAddress from;
    u8 type;
    u8 htype;
    size_t size = 0;
    char data[NET_MAX_PACKET]; // [hdr type][packtype][size][other data]
};




enum class NetError : netres_t
{
    InvalidSocket = -1,
    InvalidSize = -2,
};

#ifdef _WIN32
#define NETSOCK_INVALID 99 // Whatever windows does
#else
#define NETSOCK_INVALID -1
#endif

enum 
{
    NETSOCK_UDP = 0,
    NETSOCK_TCP = 1,
    NETSOCK_MCAST = 2,
    NETSOCK_COUNT,
};

class NetSocket
{
    public:
        int Open( const u16 port, int type );
        void Close( void ); 

        netres_t SendTo( const NetAddress& dest, const char* buff, size_t bufflen );
        netres_t RecvFrom( NetAddress* pFrom, char* buff_out, size_t bufflen );

        bool IsValid( void ) const { return m_handle != NETSOCK_INVALID; }
    private:
        netsockhandle_t m_handle = -1;
        int m_type = -1;
};

enum class ConnectionState
{
    Disconnected,
    Awaiting,
    Connected,
    Active,
};

class NetChannel
{
    public:
        NetChannel();
        const NetAddress& GetRemoteAddress( void ) const { return m_remote; };
        void SetRemoteAddress( const NetAddress& adr) { m_remote = adr; };

        u16 GetQPort( void ) const { return m_qport; }

        void SetConnectionState( ConnectionState state ) {m_state = state; };
        ConnectionState GetConnectionState( void ) const { return m_state; };

        void SetSocket( NetSocket* sock ) { m_socket = sock; };

        bool SendMessage( const char* data, size_t len );
        bool IsConnected( void ) const { return m_state == ConnectionState::Connected; }
        void ProcessPacket( NetPacket* packet );

        bool operator==( const NetChannel& other ) const 
        { return (m_remote == other.GetRemoteAddress()) && (m_qport == other.GetQPort()); }
    private:

        ConnectionState m_state;

        NetAddress m_remote;
        NetSocket* m_socket;

        u16 m_qport;

        double t_lastrecv;
        double t_lastsend;

        u32 m_outseq = 0;
        u32 m_inseq = 0; 
};


class INetLookup
{
public:
    INetLookup() = default;
    virtual ~INetLookup() = default;
    virtual NetChannel* FindNetChannel( const NetAddress& addr) = 0;
};

void UDP_ProcessSocket( NetSocket* socket, INetLookup* lookup );


bool NET_Init( void );
