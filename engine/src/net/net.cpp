#include "net/net.hpp"
#include "net/networkmanager.h"
#include "platform/plt_time.h"
#include <sys/socket.h>
#include <unistd.h>



#define ADRCST(sockadr) ( (struct sockaddr*)&sockadr) 

static inline NetAddress sockaddr_to_netaddr(struct sockaddr_in adr){
    return (NetAddress){.ip = ntohl(adr.sin_addr.s_addr), .port = ntohs(adr.sin_port)};
}

int NetSocket::Open( u16 port, int type ){
    NetAddress addr = {.ip = 0, .port = port};
    int socktype;
    switch(type){
        case NETSOCK_UDP: socktype = SOCK_DGRAM; break;
        case NETSOCK_TCP: socktype = SOCK_STREAM; break;
        default: socktype = SOCK_DGRAM; break;
    }
    m_handle = socket(AF_INET, socktype, 0);
    if (m_handle < 0){
        return false;
    }
    struct sockaddr_in sockadr = addr.SockAddr();
    int res = bind(m_handle, ADRCST(sockadr), sizeof(sockadr));
    if (res < 0){
        Close();
        return false;
    }
    m_type = type;
    return true;
}

void NetSocket::Close( void ){
    if (m_handle >= 0)
        close(m_handle);
    m_handle = -1;
}


netres_t NetSocket::SendTo( const NetAddress& dest, const char* buff, size_t bufflen )
{
    NetAddress adr = dest;
    struct sockaddr_in sockadr = adr.SockAddr();
    ssize_t len = sendto(m_handle, buff, bufflen, 0, ADRCST(sockadr), sizeof(sockadr));
    if (len < 0) {
        perror("sendto");
        return (netres_t)NetError::InvalidSize;
    }
    return static_cast<netres_t>(len);
}

netres_t NetSocket::RecvFrom( NetAddress* pFrom, char* buff_out, size_t bufflen)
{
    struct sockaddr_in from;
    socklen_t fromlen = sizeof(from);
    ssize_t len = recvfrom(m_handle, buff_out, bufflen, MSG_DONTWAIT, ADRCST(from), &fromlen);
    if (len < 0) {
        return (netres_t)NetError::InvalidSize;
    }
    *pFrom = sockaddr_to_netaddr(from);
    return static_cast<netres_t>(len);
}

NetChannel::NetChannel(){
    m_inseq = 0;
    m_outseq = 0;
    m_qport = 0;
    m_remote = NetAddress();
}

bool NetChannel::SendMessage( const char* data, size_t len ){
    ssize_t res = m_socket->SendTo(m_remote, data, len);
    return res > 0;
}



void NetChannel::ProcessPacket( NetPacket* packet ){
    // Shit, what do I do now?
    switch(packet->type){
        case (u8)PacketType::ChallengeReq:
            printf("Hello\n");
            break;
        default:
            printf("Hi\n");
            break;
    }
    m_inseq++;
}


bool UDP_ReceiveDatagram( NetSocket* socket, NetPacket* out ){
    netres_t res = socket->RecvFrom(&out->from, out->data, NET_MAX_PACKET);
    if (res <= 0) return false;
    out->size = (size_t)res;
    return true;
}

NetPacket* UDP_GetPacket( NetSocket* socket ){
    NetPacket* pack = new NetPacket();
    if (!UDP_ReceiveDatagram(socket, pack)){
        delete pack;
        return NULL;
    }
    return pack;
}

void UDP_ProcessSocket( NetSocket* socket, INetLookup* lookup )
{
    if (!socket->IsValid()) return; 


    NetPacket* inpack;

    while ( ( inpack = UDP_GetPacket( socket )) != NULL ){
        if (!g_NetworkManager->ReadPacket(inpack)){
            delete inpack;
            continue;
        } 
        if (inpack->htype != (u8)HeaderType::Connectionless){
            // Route to appropriate channel
            NetChannel* chan = lookup->FindNetChannel(inpack->from);
            if (chan)
                g_NetworkManager->EnqueuePacket( chan, inpack );
            else
                delete inpack;
        }
        else{    
            g_NetworkManager->EnqueuePacket( NULL, inpack );
        }
    }
}
