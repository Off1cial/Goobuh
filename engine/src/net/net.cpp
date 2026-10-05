#include "net/net.hpp"
#include "net/networkmanager.h"
#include "platform/plt_time.h"

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <unistd.h>
#endif

#define ADRCST(sockadr) ((struct sockaddr*)&sockadr)

static inline NetAddress sockaddr_to_netaddr(struct sockaddr_in adr)
{
    return (NetAddress){
        .ip = ntohl(adr.sin_addr.s_addr),
        .port = ntohs(adr.sin_port)
    };
}


#ifdef _WIN32

static bool InitWinsock()
{
    static bool initialized = false;

    if (initialized)
        return true;

    WSADATA wsaData;

    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (result != 0) {
        printf("WSAStartup failed: %d\n", result);
        return false;
    }

    initialized = true;
    return true;
}

#endif


int NetSocket::Open(u16 port, int type)
{
#ifdef _WIN32
    if (!InitWinsock())
        return false;
#endif

    NetAddress addr = {
        .ip = 0,
        .port = port
    };

    int socktype;

    switch (type) {
        case NETSOCK_UDP:
            socktype = SOCK_DGRAM;
            break;

        case NETSOCK_TCP:
            socktype = SOCK_STREAM;
            break;

        default:
            socktype = SOCK_DGRAM;
            break;
    }

    m_handle = socket(AF_INET, socktype, 0);

#ifdef _WIN32
    if (m_handle == INVALID_SOCKET) {
        printf("socket() failed: %d\n", WSAGetLastError());
        return false;
    }
#else
    if (m_handle < 0) {
        perror("socket");
        return false;
    }
#endif

    struct sockaddr_in sockadr = addr.SockAddr();

    int res = bind(
        m_handle,
        ADRCST(sockadr),
        sizeof(sockadr)
    );

    if (res < 0) {
#ifdef _WIN32
        printf("bind() failed: %d\n", WSAGetLastError());
#else
        perror("bind");
#endif

        Close();
        return false;
    }

#ifdef _WIN32
    // Make the socket non-blocking.
    u_long nonblocking = 1;

    if (ioctlsocket(m_handle, FIONBIO, &nonblocking) != 0) {
        printf("ioctlsocket() failed: %d\n", WSAGetLastError());
        Close();
        return false;
    }
#endif

    m_type = type;

    return true;
}


void NetSocket::Close(void)
{
#ifdef _WIN32

    if (m_handle != INVALID_SOCKET) {
        closesocket(m_handle);
        m_handle = INVALID_SOCKET;
    }

#else

    if (m_handle >= 0) {
        close(m_handle);
        m_handle = -1;
    }

#endif
}


netres_t NetSocket::SendTo(
    const NetAddress& dest,
    const char* buff,
    size_t bufflen
)
{
    NetAddress adr = dest;

    struct sockaddr_in sockadr = adr.SockAddr();

#ifdef _WIN32

    int len = sendto(
        m_handle,
        buff,
        static_cast<int>(bufflen),
        0,
        ADRCST(sockadr),
        sizeof(sockadr)
    );

    if (len == SOCKET_ERROR) {
        printf("sendto() failed: %d\n", WSAGetLastError());
        return static_cast<netres_t>(NetError::InvalidSize);
    }

#else

    ssize_t len = sendto(
        m_handle,
        buff,
        bufflen,
        0,
        ADRCST(sockadr),
        sizeof(sockadr)
    );

    if (len < 0) {
        perror("sendto");
        return static_cast<netres_t>(NetError::InvalidSize);
    }

#endif

    return static_cast<netres_t>(len);
}

bool NET_Init( void )
{
#ifdef _WIN32
    if (!InitWinsock())
        return false;
#endif
    return true;
    
}

netres_t NetSocket::RecvFrom(
    NetAddress* pFrom,
    char* buff_out,
    size_t bufflen
)
{
    struct sockaddr_in from{};

#ifdef _WIN32

    int fromlen = sizeof(from);

    int len = recvfrom(
        m_handle,
        buff_out,
        static_cast<int>(bufflen),
        0,
        ADRCST(from),
        &fromlen
    );

    if (len == SOCKET_ERROR) {
        int error = WSAGetLastError();

        // No packet available on a non-blocking socket.
        if (error == WSAEWOULDBLOCK)
            return 0;

        printf("recvfrom() failed: %d\n", error);
        return static_cast<netres_t>(NetError::InvalidSize);
    }

#else

    socklen_t fromlen = sizeof(from);

    ssize_t len = recvfrom(
        m_handle,
        buff_out,
        bufflen,
        MSG_DONTWAIT,
        ADRCST(from),
        &fromlen
    );

    if (len < 0) {
        return static_cast<netres_t>(NetError::InvalidSize);
    }

#endif

    *pFrom = sockaddr_to_netaddr(from);

    return static_cast<netres_t>(len);
}


NetChannel::NetChannel()
{
    m_inseq = 0;
    m_outseq = 0;
    m_qport = 0;
    m_remote = NetAddress();
}


bool NetChannel::SendMessage(
    const char* data,
    size_t len
)
{
    netres_t res = m_socket->SendTo(
        m_remote,
        data,
        len
    );

    return res > 0;
}


void NetChannel::ProcessPacket(NetPacket* packet)
{
    switch (packet->type) {

        case (u8)PacketType::ChallengeReq:
            printf("Hello\n");
            break;

        default:
            printf("Hi\n");
            break;
    }

    m_inseq++;
}


bool UDP_ReceiveDatagram(
    NetSocket* socket,
    NetPacket* out
)
{
    netres_t res = socket->RecvFrom(
        &out->from,
        out->data,
        NET_MAX_PACKET
    );

    if (res <= 0)
        return false;

    out->size = static_cast<size_t>(res);

    return true;
}


NetPacket* UDP_GetPacket(NetSocket* socket)
{
    NetPacket* pack = new NetPacket();

    if (!UDP_ReceiveDatagram(socket, pack)) {
        delete pack;
        return NULL;
    }

    return pack;
}


void UDP_ProcessSocket(
    NetSocket* socket,
    INetLookup* lookup
)
{
    if (!socket->IsValid())
        return;

    NetPacket* inpack;

    while ((inpack = UDP_GetPacket(socket)) != NULL) {

        if (!g_NetworkManager->ReadPacket(inpack)) {
            delete inpack;
            continue;
        }

        if (inpack->htype != (u8)HeaderType::Connectionless) {

            NetChannel* chan =
                lookup->FindNetChannel(inpack->from);

            if (chan) {
                g_NetworkManager->EnqueuePacket(
                    chan,
                    inpack
                );
            }
            else {
                delete inpack;
            }
        }
        else {
            g_NetworkManager->EnqueuePacket(
                NULL,
                inpack
            );
        }
    }
}
