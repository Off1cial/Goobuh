#include "engine/net/protocol.h"
#include "engine/net/net.h"
#include "common/common.h"


void Netchan_Transmit( 
        netchan_t* chan,
        netsock_t sock,
        int len,
        const byte *data)
{
    NET_SendData(data, len, sock, chan->remote);
}



void Netchan_GetPacket( netchan_t *chan, netsock_t sock, packet_t *out )
{
    byte buff[MAX_NETMSG + PACKET_HEADER];
    netaddr_t from;
    int res = NET_ReceiveData(
                buff, 
                MAX_NETMSG + PACKET_HEADER,
                sock,
                &from 
            );
    if (!NET_CompareAddreses(from, chan->remote)){
        Con_printf("Netchan: inbound packet (%d) from non-remote address: %s\n", res, NET_AddrToString(from)); 
        out->length = 0;
        out->data = NULL;
        out->type = PKT_UNKNOWN;
        // What to do with unknown packet?
        // The client or server shouldn't really get here..
        return;
    }
    memcpy(out->data, buff + PACKET_HEADER, res - PACKET_HEADER);
    out->type = buff[0];
    out->length = res - PACKET_HEADER;
}


