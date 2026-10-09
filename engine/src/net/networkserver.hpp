#pragma once

#include "net/net.hpp"
#include <vector>
// A client, seen from the server
class INetServerClient;

class INetServer : public INetLookup
{
    public:
        // Non-virtual, guaranteed behaviour
        virtual ~INetServer() = default;
        virtual bool Init( u16 port = NET_SERVER_DEFAULT_PORT );
        virtual void Shutdown( void );
        
        virtual void ChallengeConnection ( const NetAddress& remote ); // Sends the challenge
        virtual void AcceptConnection( INetServerClient* client );
        virtual void OnConnectionClose( const NetChannel& chan, const char* reason);

        virtual void ReadPackets( void );

        // Used to initiate a channel for sending back a challenge
        virtual INetServerClient* TempClient( const NetAddress& remote );
        // We recieved a correct challenge in response, let them join
        virtual void AuthoriseClient( INetServerClient* client );


        virtual INetServerClient* FindClientByAddress( const NetAddress& addr );
        virtual INetServerClient* FindClientByChannel( const NetChannel& channel );
        
        // INetLookup
        virtual NetChannel* FindNetChannel( const NetAddress& addr ) override;

        u32 GetTickrate( void ) const {return m_tickrate; };
        void SetTickrate( u32 tick) { m_tickrate = tick; };
    
        u32 m_tickrate;
        float m_tickrate_interval;
        challenge_t m_challenge;

        NetSocket m_sockudp;
        std::vector<INetServerClient*> m_clients;

};


class INetServerClient
{
    public:
        virtual ~INetServerClient() = default;
        virtual void SendUpdate( void ) {};
        virtual const char* GetName( void ) { return "GoonLord"; };

        NetChannel m_chan;
        bool m_remove = false;
};
