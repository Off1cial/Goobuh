#include "engine/server/server.h"

#define SV_ARENA_SIZE 1024 * 1024 * 1024

server_t sv = {.state = SS_DEAD};
sysarena_t sv_arena;

bool SV_Init( char name[64], int maxplayers )
{
    if (!netsock_open( &sv.udpsocket, SERVER_PORT )) return false;

    if (!Sys_ArenaInit( &sv_arena, SV_ARENA_SIZE, "SvArena", MEMTAG_SERVER )) return false;

    sv.clients = SV_Alloc( sizeof(svclient_t) * maxplayers );
    if (!sv.clients) return false;

    sv.state = SS_ACTIVE;
    return true;
}

void SV_Shutdown( void )
{
// Drop all clients
// Close socket
// Engine handles the arena
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