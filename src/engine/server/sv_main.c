#include "common/common.h"
#include "engine/server/server.h"

#define SV_ARENA_SIZE 1024 * 1024 * 1024

server_t sv = {.state = SS_DEAD, .clients = NULL, .numclients = 0};
svclient_t* svcl = NULL;
sysarena_t sv_arena;

bool SV_Init( int maxplayers )
{
    
    if (!netsock_open( &sv.udpsocket, SERVER_PORT ))
    {
        Con_printf("[Server]: Failed to initialise, could not open socket\n");
        return false;
    }

    if (!Sys_ArenaInit( &sv_arena, SV_ARENA_SIZE, "SvArena", MEMTAG_SERVER ))
    {
        Con_printf("[Server]: Failed to initialise, could not create arena\n");
        return false;
    }

    sv.clients = SV_Alloc( sizeof(svclient_t) * maxplayers );
    if (!sv.clients) return false;

    sv.client_func[SSCL_ZOMBIE] = SV_HandleZombieClient; 
    sv.client_func[SSCL_CONNECTED] = SV_HandleConnectedClient; 
    sv.client_func[SSCL_ACTIVE] = SV_HandleActiveClient; 

    sv.state = SS_ACTIVE;
    
    

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


void SV_ReadPackets( void )
{
    svcl = sv.clients;
    while (NET_GetPacket() )
    {
        for ( int i = 0; i < sv.maxclients; i++)
        {
            if (svcl->state = SSCL_DEAD) continue;


            if (!NET_CompareAddresses( net_from, svcl->chan.remote ))
                continue;
            

            if (svcl->state != SSCL_ZOMBIE)
                SV_ExecuteClientMessage( svcl );


            svcl++;
        }
    }

}


void SV_Think( void )
{
    SV_ReadPackets();
}




void SV_DropClient( svclient_t* client, FORMAT_STR )
{
    if (!client) return;

    // Send disconnection packet


    // Clear index
    memset(client, 0, sizeof(svclient_t));
    client->state = SSCL_DEAD;
}

