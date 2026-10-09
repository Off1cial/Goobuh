#pragma once

#include "net/net.hpp"
#include "net/bitbuff.hpp"

#include <queue>

#ifndef NET_MAX_STRING
#define NET_MAX_STRING 256
#endif

class INetServer;
class INetClient;

// Manages a client/server, or both
class CNetworkManager 
{
public:

    ~CNetworkManager( void ) { Shutdown(); }
    bool Init( void );
    void Shutdown( void );

    void Update( void );

    // can be both e.g local server whilst playing
    bool IsServer( void ) const { return m_isServer; };
    bool IsClient( void ) const { return m_isClient; };
    
    INetServer* StartServer( u16 port = NET_SERVER_DEFAULT_PORT); 
    void ShutdownServer( void );
    bool SendServerMessage( const char* data, size_t len, PacketType type, NetChannel* pChan );
    bool SendServerUnconnectedMessage( const char* data, size_t len, PacketType type, NetChannel* chan);
    bool SendServerChallenge( NetChannel* pChan );

    INetClient* StartClient( u16 port = NET_CLIENT_DEFAULT_PORT);
    void ShutdownClient( void );
    void ConnectClient( const char* ip, u16 port );
    bool SendClientMessage( const char* data, size_t len, PacketType type );
    bool SendClientChallenge( void );

    // Fills the packet struct/header from the inbound data
    bool ReadPacket( NetPacket* packet );

    void EnqueuePacket( NetChannel* chan,  NetPacket* pack );
    //void EnqueueConnectionlessPacket( NetPacket* pack );

    void ProcessNewPacket( void );
    
    // Raw data sending

    const char* GetLocalHostName( void );
    NetAddress GetLocalAddress( void );

private:
    
    struct PacketInfo_t
    {
        NetPacket* pack; // delete when processed by recipient?
        NetChannel* chan; 
    };

    void ProcessConnectionlessPacket( PacketInfo_t& pack );

    void ProcessClientMessages( void );
    void ProcessServerMessages( void );



    bool SendConnectionless( const NetAddress& dest, NetSocket& sock, PacketType type, const char* data, size_t datalen );

    void FindLocalAddress( void );


    bool m_isClient;
    bool m_isServer;

    INetServer* m_server;
    INetClient* m_client;

    u16 m_serverport;
    u16 m_clientport;

    u32 m_clienttickrate;
    u32 m_servertickrate;

    char m_LocalHostName[NET_MAX_STRING];
    NetAddress m_localAddress;

    std::queue<PacketInfo_t> m_qPackets;
    //std::queue<NetPacket*> m_qPacketsNoConn;  // Connectionless packets

    // Tools
    bf_read m_read;
    bf_write m_write;
};

extern CNetworkManager* g_NetworkManager;
