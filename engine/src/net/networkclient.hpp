#pragma once
#include "net/net.hpp"



class CNetClient : public INetLookup
{
public:
    CNetClient() = default;
    ~CNetClient() = default;
    bool Init( u16 port = NET_CLIENT_DEFAULT_PORT );
    void Shutdown();

    NetChannel* FindNetChannel( const NetAddress& addr);


    void Connect( const char* ip, u16 port );
    void Disconnect( void );

    bool SendPacket( PacketType type, const char* data, size_t datalen );
    void ReadPackets( void );

    bool SendConnectionReq( void );


    u32 GetTickrate( void ) const {return m_tickrate; };
    void SetTickrate( u32 tick) { m_tickrate = tick; };

    challenge_t m_challenge;
    u32 m_tickrate;
    NetSocket m_udpsocket;
    NetChannel m_chan;



};
