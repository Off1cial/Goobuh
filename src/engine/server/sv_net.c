#include "engine/net/net.h"
#include "engine/server/server.h"


static inline svclient_t* SV_ClientByAddress( netaddr_t adr )
{
    int c;
    for (c = 0; c < sv.maxclients; c++)
    {
        if ( NET_CompareAddresses( adr, sv.clients[c].chan.remote ))
            return &sv.clients[c];
    }
    return NULL;
}



void SV_ClientSendChallenge( svclient_t* client )
{
    union 
    {
        int c;
        byte b[4];
    } buf;
    buf.c = htonl(sv.challenge.challenge);
    Netchan_Transmit( &client->chan, sv.udpsocket, 4, buf.b );
}





