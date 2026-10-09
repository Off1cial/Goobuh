#include "net/networkserver.hpp"
#include "net/net.hpp"

bool INetServer::Init( u16 port ){
    if (!m_sockudp.Open(port, NETSOCK_UDP)) return false;
    // For now
    m_tickrate = 32;
    m_tickrate_interval = 1.0f / m_tickrate;
    m_challenge = {.dest = NetAddress(), .challenge = 69, .duration = 3.0f};
    return true;
}

void INetServer::OnConnectionClose( const NetChannel& chan, const char* reason ){
    m_sockudp.SendTo( chan.GetRemoteAddress(), reason, strlen(reason));
}

void INetServer::Shutdown(){
    for (auto client : m_clients){
        OnConnectionClose( client->m_chan, "Server shutting down");
    }
    m_sockudp.Close();
}

NetChannel* INetServer::FindNetChannel( const NetAddress& addr )
{
    for (INetServerClient* cl : m_clients)
    {
        if (cl->m_chan.GetRemoteAddress() == addr){
            return &cl->m_chan;
        }
    }
    return NULL;
}

INetServerClient* INetServer::FindClientByAddress( const NetAddress& addr ){
    for (INetServerClient* cl : m_clients){
        if (cl->m_chan.GetRemoteAddress() == addr){
            return cl;
        }
    }
    return NULL;
}

INetServerClient* INetServer::FindClientByChannel( const NetChannel& channel )
{
    for (INetServerClient* cl : m_clients)
    {
        if (cl->m_chan == channel){
            return cl;
        }
    }
    return NULL;
}



void INetServer::ReadPackets( void ){
    UDP_ProcessSocket( &m_sockudp, this ); 
    auto s = m_clients.begin();
    for (int i = m_clients.size() - 1; i >= 0; i--){
        INetServerClient* cl = m_clients[i];
        if (!cl) continue;
        if (cl->m_remove){
            m_clients.erase(s + i);
            delete cl;
        }
    }
}


INetServerClient* INetServer::TempClient( const NetAddress& remote ){
    INetServerClient* cl = new INetServerClient;
    cl->m_chan.SetRemoteAddress(remote);
    cl->m_chan.SetConnectionState(ConnectionState::Awaiting);
    cl->m_chan.SetSocket(&m_sockudp);
    m_clients.push_back(cl);
    return cl;
}

void INetServer::AuthoriseClient( INetServerClient* client ){
    if (!client) return;
    client->m_chan.SetConnectionState(ConnectionState::Connected);    
}


void INetServer::AcceptConnection( INetServerClient* client )
{
   AuthoriseClient( client ); 
}

void INetServer::ChallengeConnection ( const NetAddress& remote ){

}


