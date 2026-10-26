#include "engine/client/client.h"

#include "engine/client/cl_input.h"
#include "engine/rendering/renderer.h"

clientstate_t cl = {0};
sysarena_t cl_arena = {0};

SZ_NEW( cl_sizebuff, END_LITTLE );

bool CL_Init( void )
{
    SZ_Alloc( &cl_sizebuff, 512, MEMTAG_CLIENT );
    if (!Sys_ArenaInit( &cl_arena, CL_ARENA_SIZE, "ClientArena", MEMTAG_CLIENT )) return false;

    return true;
}

void CL_Shutdown( void );

void CL_Frame( void )
{
    IN_Update();

    R_Start( cl.viewangles );

    R_Present();

}


