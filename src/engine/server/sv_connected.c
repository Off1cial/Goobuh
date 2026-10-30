#include "engine/server/server.h"


void SV_HandleConnectedClient( svclient_t *client, packet_t* latest )
{ 
    Con_printf("Received %dB from client\n", latest->length );
}

// Snapshots, acting commands etc
