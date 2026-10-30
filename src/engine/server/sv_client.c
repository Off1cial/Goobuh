#include "engine/server/server.h"



void SV_HandleZombieClient( svclient_t* client, packet_t *latest )
{
}
void SV_HandleActiveClient( svclient_t* client, packet_t *latest )
{
    Con_printf("Received %dB from client\n", latest->length );
}
