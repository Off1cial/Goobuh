#include "engine/net/protocol.h"
#include "engine/net/net.h"
#include "common/common.h"





bool Netchan_Process( netchan_t *chan )
{
    if (!NET_CompareAddresses(net_from, chan->remote))
        return false;

    // Sequencing....
    long sequence, ack, len;
    sequence = SZ_ReadLong( &net_message, 0, &sequence ) ? sequence : -1;
    len = SZ_ReadLong( &net_message, sizeof(long) * 2, &len ) ? len : -1;
    //ack = SZ_ReadLong( &net_message, sizeof(long)* 2, &ack ) ? ack : -1;
    if ( (sequence < 0) || (len < 0) ) return false;
        
    chan->incoming_sequence = sequence;
    Con_printf("Netchan_Process: incoming_sequence = %ld, len = %ld\n", sequence, len);
    // End
    return true;
}



void Netchan_SendPacket( netchan_t *chan, netsock_t sock, int len, const byte *data )
{
    SZ_WriteLong( &net_message, chan->outgoing_sequence++ );
    SZ_WriteLong( &net_message, len );
    SZ_WriteBytes( &net_message, data, len );

    NET_SendData( net_message.data, net_message.cursize, sock, chan->remote );
    SZ_Clear( &net_message );
}
