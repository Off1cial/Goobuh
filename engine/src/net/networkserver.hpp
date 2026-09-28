#pragma once

#include "net/net.hpp"
#include <vector>
// A client, seen from the server
class CNetServerClient;

class CNetServer : public INetLookup
{
    public:
        // Non-virtual, guaranteed behaviour
        virtual ~CNetServer() = default;
        bool Init( u16 port = NET_SERVER_DEFAULT_PORT );
        void Shutdown( void );
        
        void AcceptConnection( const NetAddress& remote );
        void ChallengeConnection ( const NetAddress& remote ); // Sends the challenge
        virtual void OnConnectionClose( const NetChannel& chan, const char* reason);

        void ReadPackets( void );

        // Used to initiate a channel for sending back a challenge
        CNetServerClient* TempClient( const NetAddress& remote );
        // We recieved a correct challenge in response, let them join
        void AuthoriseClient( CNetServerClient* client );


        CNetServerClient* FindClientByAddress( const NetAddress& addr );
        CNetServerClient* FindClientByChannel( const NetChannel& channel );
        
        // INetLookup
        NetChannel* FindNetChannel( const NetAddress& addr ) override;

        u32 GetTickrate( void ) const {return m_tickrate; };
        void SetTickrate( u32 tick) { m_tickrate = tick; };
    
        u32 m_tickrate;
        challenge_t m_challenge;

        NetSocket m_sockudp;
        std::vector<CNetServerClient*> m_clients;

};


class CNetServerClient
{
    public:
        virtual ~CNetServerClient() = default;
        virtual void SendUpdate( void ) {};
        virtual const char* GetName( void ) { return "GoonLord"; };

        NetChannel m_chan;
        bool m_remove = false;
};
