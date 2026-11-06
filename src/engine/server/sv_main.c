#include "common/common.h"
#include "engine/net/net.h"
#include "engine/net/protocol.h"
#include "engine/server/server.h"
#include "engine/net/msg.h"
#include <stdarg.h>
#include <stdio.h>

#define SV_ARENA_SIZE 1024 * 1024 * 1024

server_t sv = {.state = SS_DEAD, .clients = NULL, .numclients = 0};
svclient_t* svcl = NULL;
sysarena_t sv_arena;

static long ticktime = 0;

bool SV_Init( const char* name, int maxplayers )
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

    sv.maxclients = maxplayers;
    sv.clients = SV_Alloc( sizeof(svclient_t) * maxplayers );
    if (!sv.clients) return false;
    
    for (int c = 0; c < sv.maxclients; c++)
    {
        sv.clients[c].state = SSCL_DEAD;
    }

    sv.client_func[SSCL_ZOMBIE] = SV_HandleZombieClient; 
    sv.client_func[SSCL_CONNECTED] = SV_HandleConnectedClient; 
    sv.client_func[SSCL_ACTIVE] = SV_HandleActiveClient; 

    sv.state = SS_ACTIVE;
    Q_strncpy( sv.name, name, sizeof(sv.name) ); 
    sv.mapname[0] = '\0';
    
    SV_InitMaps();
    SV_PrintStatus();

    return true;
}


void SV_PrintStatus( void )
{
    Con_printf("%s\n", sv.name);
    Con_printf("Map: %s\n", sv.mapname );
    Con_printf("%d/%d players\n", sv.numclients, sv.maxclients);

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
    short qport = -1;
    svcl = sv.clients;
    while (NET_GetPacket() )
    {
        qport = -1;
        MSG_ReadShort(0, &qport);

        for ( int i = 0; i < sv.maxclients; i++, svcl++)
        {
            if (svcl->state == SSCL_DEAD) 
                continue;
            
            // Wrong client on network 
            if (qport != svcl->chan.qport)
                continue;
            // Wrong formatting
            if (!Netchan_Process( &svcl->chan ))
                continue;

            if (svcl->state != SSCL_ZOMBIE)
                SV_ExecuteClientMessage(svcl);

        }
    }
}

long lastrun = 0;
void SV_Frame( void )
{
    long now = Sys_TimeMillis();
    long dt = 1000 * (now - lastrun);
    if (dt < (1.0f / 64.0f )) return;
    lastrun = now;

    ticktime = now;
    SV_CheckTimeouts();
    SV_ReadPackets();
}



void SV_CheckTimeouts( void )
{
    svclient_t *cl = sv.clients;
    for (int i = 0; i < sv.maxclients; i++, cl++)
    {
        if (cl->state == SSCL_DEAD)
            continue;
    
        if (ticktime - cl->chan.t_lastrecv > CHAN_TIMEOUT)
        {
            SV_DropClient(cl, "Client timed out");
        }
    }
}


void SV_DropClient( svclient_t* client, FORMAT_STR )
{
    if (!client) return;

    va_list args;
    va_start(args, str);

    va_list copy;
    va_copy(copy, args);


    long len = vsnprintf(NULL, 0, str, copy);
    va_end(copy);

    SV_Mark();
    char* buf = SV_Alloc( len + 1 );
    vsnprintf(buf, len + 1, str, args );
    va_end(args);
    
    // Send disconnection packet
    Netchan_SendPacket( &client->chan, sv.udpsocket, len + 1, buf);

    SV_FreeToMark();
    // Clear index
    memset(client, 0, sizeof(svclient_t));
    client->state = SSCL_DEAD;
}

