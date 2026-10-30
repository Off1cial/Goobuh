#include "common/common.h"
#include "engine/server/server.h"

#define SV_ARENA_SIZE 1024 * 1024 * 1024

server_t sv = {.state = SS_DEAD, .clients = NULL, .numclients = 0};
svclient_t* svcl = NULL;
sysarena_t sv_arena;

bool SV_Init( char name[64], int maxplayers )
{
    
    if (!netsock_open( &sv.udpsocket, SERVER_PORT )) return false;

    if (!Sys_ArenaInit( &sv_arena, SV_ARENA_SIZE, "SvArena", MEMTAG_SERVER )) return false;

    sv.clients = SV_Alloc( sizeof(svclient_t) * maxplayers );
    if (!sv.clients) return false;

    sv.client_func[SSCL_ZOMBIE] = SV_HandleZombieClient; 
    sv.client_func[SSCL_CONNECTED] = SV_HandleConnectedClient; 
    sv.client_func[SSCL_ACTIVE] = SV_HandleActiveClient; 

    sv.state = SS_ACTIVE;
    
    Q_strncpy( sv.name, name, 64 );

    Con_printf("[Server]: Server initialised\n");
    return true;
}

void SV_Shutdown( void )
{
// Drop all clients
// Close socket
    Sys_ArenaDestroy( &sv_arena);
    netsock_close( &sv.udpsocket );
    sv.state = SS_DEAD;
}

void* SV_Alloc( size_t size )
{
    return Sys_ArenaAlloc( &sv_arena, size );
}


void SV_Mark( void )
{
    Sys_ArenaMark( &sv_arena );
}
void SV_FreeToMark( void )
{
    Sys_ArenaFreeToMark( &sv_arena );
}


void SV_Think( void )
{
    int c;
    for (c = 0; c < sv.maxclients; c++)
    {
        svcl = &sv.clients[c];
        if (svcl->state == SSCL_DEAD) continue;
    
        long now = Sys_TimeMillis();

        if (now - svcl->chan.t_lastrecv >= CHAN_TIMEOUT)
        {
            SV_DropClient( svcl );
            continue;
        }


        packet_t thispacket = {0};
        while (Netchan_GetPacket( &svcl->chan, sv.udpsocket, &thispacket ) > 0)
        {
            // Apply 
            sv.client_func[svcl->state]( svcl, &thispacket ); 
        }
    }
}




void SV_DropClient( svclient_t* client )
{
    if (!client) return;

    memset(client, 0, sizeof(svclient_t));
    client->state = SSCL_DEAD;
}
