#include "net/networkserver.hpp"
#include "net/networkclient.hpp"
#include "net/networkmanager.h"
#include "net/net.hpp"

#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "platform/plt_time.h"


CNetworkManager* g_NetworkManager = NULL;

static NetAddress netaddr_getnet(u16 port)
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in remote = {
        .sin_family = AF_INET,
        .sin_port = htons(9),
        .sin_addr.s_addr = 0
    };

    inet_pton(AF_INET, "1.1.1.1", &remote.sin_addr);
    int con = connect(sock, (struct sockaddr*)&remote, sizeof(remote));

    if (con < 0){
        printf("failed con\n");
        return NetAddress();
    }
    struct sockaddr_in local;
    socklen_t len = sizeof(local);

    getsockname(
        sock,
        (struct sockaddr*)&local,
        &len
    );
    
    close(sock);
    NetAddress netaddr;
    netaddr.ip  = ntohl(local.sin_addr.s_addr);
    netaddr.port = port;
    return netaddr;
}



bool CNetworkManager::Init(){
    m_isClient = m_isServer = false;
    m_server = NULL;
    m_client = NULL;

    
    gethostname(m_LocalHostName, NET_MAX_STRING);

    m_localAddress = netaddr_getnet(0); 
    char hostip[256];

    printf("NetworkManager\n[Hostname] %s\n[Host IP]: %s\n", 
            m_LocalHostName, m_localAddress.ToString(hostip, 256));

    return true;
}

void CNetworkManager::Update(){
    if (m_isClient){
        ProcessClientMessages();
    }
    if (m_isServer){
        ProcessServerMessages();
    }
    ProcessNewPacket();
}


bool CNetworkManager::StartServer( u16 port )
{
    m_server = new CNetServer;
    m_isServer = m_server->Init(port);
    m_serverport = port;
    m_servertickrate = m_server->GetTickrate();
    return m_isServer;
}

bool CNetworkManager::StartClient( u16 port ){
    m_client = new CNetClient;
    m_isClient = m_client->Init(port);
    m_clientport = port;
    m_clienttickrate = m_client->GetTickrate();
    return m_isClient;
}

void CNetworkManager::ShutdownServer(){
    m_server->Shutdown();
    m_isServer = false;
    delete m_server;
}

void CNetworkManager::ShutdownClient(){
    m_client->Shutdown();
    m_isClient = false;
    delete m_client;
}

void CNetworkManager::Shutdown(){
    if (m_isServer) ShutdownServer();
    if (m_isClient) ShutdownClient();
}


void CNetworkManager::ConnectClient( const char* ip, u16 port ){
    m_client->Connect(ip, port);
}

bool CNetworkManager::ReadPacket(NetPacket* packet) {
    if (packet->size < 2)
        return false;
    size_t pos = 0;
    m_read.SetBuff(packet->data, packet->size);
    if (!m_read.ReadU8At(&packet->htype, &pos)) return false;
    if (!m_read.ReadU8At(&packet->type, &pos)) return false;
        
    packet->size -= 2;
    memmove(packet->data, packet->data + 2, packet->size);

    return true;
}

u32 ReadPacketChallenge( bf_read& read, NetPacket* packet ){
    read.SetBuff(packet->data, packet->size);
    u32 out;
    size_t pos = 0;
    read.ReadU32At( &out, &pos );
    return out;
}

void CNetworkManager::EnqueuePacket( NetChannel* chan, NetPacket* pack ){
    PacketInfo_t info;
    info.chan = chan;
    info.pack = pack;
    m_qPackets.push(info);
}




void CNetworkManager::ProcessNewPacket( void ){
    while (m_qPackets.size() > 0){
        PacketInfo_t info = m_qPackets.front();

        // Process
        if (info.chan){ 
            info.chan->ProcessPacket( info.pack );
        }else{
            // Do connectionless stuff, 
            ProcessConnectionlessPacket( info );
        }
        delete info.pack;
        m_qPackets.pop();
    }
}



void CNetworkManager::ProcessConnectionlessPacket( PacketInfo_t& pack ){
    char buf[256];
    pack.pack->from.ToString(buf, 256);
    m_read.SetBuff(pack.pack->data, pack.pack->size);
    u32 challenge_val = 0;
    switch((PacketType)pack.pack->type){
        case PacketType::ChallengeReq:
            printf("Challenge request\n");
            if (m_isServer){
                CNetServerClient* cl = m_server->FindClientByAddress( pack.pack->from );
                if (cl) break;

                cl = m_server->TempClient( pack.pack->from );

                // Do server things, send the client our challenge
                printf("Received challenge req from %s\n", buf);
                /*
                if (!SendServerUnconnectedMessage(nullptr, 0, PacketType::ChallengeSend, &cl->m_chan)){
                    printf("Failed to send back challenge\n");
                }else{
                    printf("Sent back challenge\n");
                }
                */
                SendServerChallenge(&cl->m_chan);
            }
            break;
        case PacketType::ChallengeSend:
            printf("Challlenge received from %s\n", buf);
            challenge_val = ReadPacketChallenge(m_read, pack.pack);
            if (m_isClient){
                // Send back the challenge
                m_client->m_challenge.dest = pack.pack->from;
                m_client->m_challenge.challenge = challenge_val;
            }
            if (m_isServer){
                // Compare with what we sent
                // if true, accept client, otherwise send denial
                if (challenge_val == m_server->m_challenge.challenge){
                    printf("Approved\n");
                }
            }
            break;
        case PacketType::ChallengeApprove:
            printf("Challenge approved\n");
            if (m_isClient){
                // We had our challenge approved, join the server
            }
            break;
        case PacketType::ChallengeDenied:
            printf("Challenge denied\n");
            if (m_isClient){
                // Set our state back to idle, we couldn't join
            }
            break;
    }
}

bool CNetworkManager::SendClientMessage( const char* data, size_t len, PacketType type ){
    if (NET_MAX_PACKET - 2 <= len) return false;
    char buff[NET_MAX_PACKET];
    buff[0] = (m_client->m_chan.IsConnected()) ? (u8)HeaderType::Connected : (u8)HeaderType::Connectionless;
    buff[1] = (u8)type;
    if (data) memcpy(buff +2, data, len);
    bool res = m_client->m_chan.SendMessage( buff, len + 2 );
    return res;
}

bool CNetworkManager::SendServerMessage( const char* data, size_t len, PacketType type, NetChannel* pChan){
    if (!pChan) return false;

    if (NET_MAX_PACKET - 2 <= len) return false;
    char buff[NET_MAX_PACKET];
    buff[0] = (pChan->IsConnected()) ? (u8)HeaderType::Connected : (u8)HeaderType::Connectionless;
    buff[1] = (u8)type;
  
    if (data) memcpy(buff + 2, data, len);
    bool res = pChan->SendMessage( buff, len + 2 );
    return res;
}


bool CNetworkManager::SendServerChallenge( NetChannel* pChan ){
    char  buff[NETCHALLENGE_SIZE]; 
    m_write.SetBuff(buff, NETCHALLENGE_SIZE);
    size_t pos = 0;
    m_write.WriteU32At(m_server->m_challenge.challenge, &pos );
    return SendServerUnconnectedMessage( buff, NETCHALLENGE_SIZE, PacketType::ChallengeSend, pChan );
}


bool CNetworkManager::SendServerUnconnectedMessage( const char* data, size_t len, PacketType type, NetChannel* chan){
   if (NET_MAX_PACKET - 2 <= len) return false;
   char buff[NET_MAX_PACKET];
   buff[0] = (u8)HeaderType::Connectionless;
   buff[1] = (u8)type;
   if (data) memcpy(buff + 2, data, len);
   //bool res = m_server->m_sockudp.SendTo(addr, buff, len + 2);
   bool res = chan->SendMessage( buff, len + 2);
   return res;

}

void CNetworkManager::ProcessServerMessages( void ){
    m_server->ReadPackets();
    // Do things
}


void CNetworkManager::ProcessClientMessages( void ){
    m_client->ReadPackets();
}
