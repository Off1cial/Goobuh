#include "net/networkclient.hpp"
#include "net/networkmanager.h"
#include <cstdio>
#include <unistd.h>



bool INetClient::Init( u16 port ){
    if (!m_udpsocket.Open( port, NETSOCK_UDP )) return false;
    m_tickrate = 32;
    m_chan.SetSocket(&m_udpsocket);
    return true;
}

void INetClient::Shutdown(){
    m_udpsocket.Close();
}

bool INetClient::SendPacket( PacketType type, const char* data, size_t datalen ){
   return g_NetworkManager->SendClientMessage( data, datalen, type ); 
}

bool INetClient::SendConnectionReq( void ){
    return SendPacket(PacketType::ChallengeReq, nullptr, 0);
}

void INetClient::ReadPackets( void ){
    UDP_ProcessSocket(&m_udpsocket, this);

    
}


NetChannel* INetClient::FindNetChannel( const NetAddress& addr ){
    if (m_chan.GetRemoteAddress() != addr)
        return NULL;
    return &m_chan;
}

void INetClient::Connect( const char* ip, u16 port ){
    NetAddress remote;
    remote = remote.FromString(ip, port);
    m_chan.SetRemoteAddress(remote); 
    // Send conncetion request packet to server
    if (SendConnectionReq()){
        m_chan.SetConnectionState(ConnectionState::Awaiting); 
    }
    else{
        m_chan.SetConnectionState(ConnectionState::Disconnected);
    }
}

void INetClient::Disconnect( const char* reason, ... ){
    m_chan.SetConnectionState(ConnectionState::Disconnected);

    SendPacket(PacketType::ChallengeDenied, nullptr, 0);
}
