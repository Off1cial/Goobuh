#include "engine/net/net.h"
#include "engine/net/protocol.h"
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



void SV_SendChallenge( svclient_t* client )
{
    byte msg[5];
    union 
    {
        int c;
        byte b[4];
    } buf;
    buf.c = htonl(sv.challenge.challenge);
    msg[0] = PKT_CHALLENGE_SEND;
    msg[1] = buf.b[1];
    msg[2] = buf.b[2];
    msg[3] = buf.b[3];
    msg[4] = buf.b[4];
    Netchan_SendPacket( &client->chan, sv.udpsocket, 4, msg);
}





