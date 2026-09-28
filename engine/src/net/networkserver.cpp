#include "net/networkserver.hpp"
#include "net/net.hpp"

bool CNetServer::Init( u16 port ){
    if (!m_sockudp.Open(port, NETSOCK_UDP)) return false;
    // For now
    m_tickrate = 32;
    m_challenge = {.dest = NetAddress(), .challenge = 69, .duration = 3.0f};
    return true;
}

void CNetServer::OnConnectionClose( const NetChannel& chan, const char* reason ){
    m_sockudp.SendTo( chan.GetRemoteAddress(), reason, strlen(reason));
}

void CNetServer::Shutdown(){
    for (auto client : m_clients){
        OnConnectionClose( client->m_chan, "Server shutting down");
    }
    m_sockudp.Close();
}

NetChannel* CNetServer::FindNetChannel( const NetAddress& addr )
{
    for (CNetServerClient* cl : m_clients)
    {
        if (cl->m_chan.GetRemoteAddress() == addr){
            return &cl->m_chan;
        }
    }
    return NULL;
}

CNetServerClient* CNetServer::FindClientByAddress( const NetAddress& addr ){
    for (CNetServerClient* cl : m_clients){
        if (cl->m_chan.GetRemoteAddress() == addr){
            return cl;
        }
    }
    return NULL;
}

void CNetServer::ReadPackets( void ){
    UDP_ProcessSocket( &m_sockudp, this ); 
    auto s = m_clients.begin();
    for (int i = m_clients.size() - 1; i >= 0; i--){
        CNetServerClient* cl = m_clients[i];
        if (!cl) continue;
        if (cl->m_remove){
            m_clients.erase(s + i);
            delete cl;
        }
    }
}


CNetServerClient* CNetServer::TempClient( const NetAddress& remote ){
    CNetServerClient* cl = new CNetServerClient;
    cl->m_chan.SetRemoteAddress(remote);
    cl->m_chan.SetConnectionState(ConnectionState::Awaiting);
    cl->m_chan.SetSocket(&m_sockudp);
    m_clients.push_back(cl);
    return cl;
}

void CNetServer::AuthoriseClient( CNetServerClient* client ){
    if (!client) return;
    client->m_chan.SetConnectionState(ConnectionState::Connected);    
}



void CNetServer::ChallengeConnection ( const NetAddress& remote ){

}

