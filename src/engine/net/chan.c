#include "engine/net/protocol.h"
#include "engine/net/net.h"
#include "common/common.h"
#include "engine/shared/cvar.h"
#include <time.h>

cvar_t cv_qport = {.name = "qport", .string = "0", .value = 0};




bool Netchan_Process( netchan_t *chan )
{
    if (!NET_CompareAddresses(net_from, chan->remote))
        return false;

    // Sequencing....
    long sequence, ack, len;

    long seq_start = sizeof(short) + 1 + sizeof(long);
    long len_start = sizeof(short) + 1;

    sequence = SZ_ReadLong( &net_message, seq_start, &sequence ) ? sequence : -1;
    len = SZ_ReadLong( &net_message, len_start, &len ) ? len : -1;
    if ( (sequence < 0) || (len < 0) ) return false;
        
    chan->incoming_sequence = sequence;
    Con_printf("Netchan_Process: incoming_sequence = %ld, len = %ld\n", sequence, len);
    // End
    return true;
}



void Netchan_SendPacket( netchan_t *chan, netsock_t sock, int len, const byte *data )
{
    SZ_WriteShort( &net_message, chan->qport);
    SZ_WriteLong( &net_message, len );
    SZ_WriteLong( &net_message, chan->outgoing_sequence++ );
    SZ_WriteBytes( &net_message, data, len );

    NET_SendData( net_message.data, net_message.cursize, sock, chan->remote );
    SZ_Clear( &net_message );
}


void Netchan_Setup( netchan_t *chan )
{
    unsigned short port;
            // 0xffff
    port = ((short)-1) & ((Sys_TimeMillis() * 1000) * time(NULL) );
    chan->incoming_sequence = 0;
    chan->outgoing_sequence = 0;
    chan->qport = port;
    chan->remote = (netaddr_t){0};
    chan->t_lastrecv = 0;
    chan->t_lastsend = 0;

    Cvar_Register (&cv_qport);
    Con_printf("qport=  %d\n", port);
}


void Netchan_ResetSequences( netchan_t* chan )
{
    chan->incoming_sequence = 0;
    chan->outgoing_sequence = 0;
    chan->t_lastrecv = 0;
    chan->t_lastsend = 0;
}
