#pragma once
#include "net/net.hpp"



class INetClient : public INetLookup
{
public:
    INetClient() = default;
    virtual ~INetClient() = default;
    virtual bool Init( u16 port = NET_CLIENT_DEFAULT_PORT );
    virtual void Shutdown();

    virtual NetChannel* FindNetChannel( const NetAddress& addr);


    virtual void Connect( const char* ip, u16 port );
    virtual void Disconnect( const char* reason, ... );

    virtual bool SendPacket( PacketType type, const char* data, size_t datalen );
    virtual void ReadPackets( void );

    virtual bool SendConnectionReq( void );


    virtual u32 GetTickrate( void ) const {return m_tickrate; };
    virtual void SetTickrate( u32 tick) { m_tickrate = tick; };

    challenge_t m_challenge;
    u32 m_tickrate;
    NetSocket m_udpsocket;
    NetChannel m_chan;



};
