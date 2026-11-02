#include "engine/client/client.h"

#include "engine/client/cl_input.h"
#include "engine/client/pmove.h"
#include "engine/rendering/renderer.h"
#include "engine/net/net.h"
#include "engine/net/msg.h"
#include "engine/map/bsp.h"

clientstate_t cl = {0};
sysarena_t cl_arena = {0};

SZ_NEW( cl_sizebuff, END_LITTLE );

bool CL_Init( void )
{
    SZ_Alloc( &cl_sizebuff, 512, MEMTAG_CLIENT );
    if (!Sys_ArenaInit( &cl_arena, CL_ARENA_SIZE, "ClientArena", MEMTAG_CLIENT )) return false;

    if (!netsock_open(&cl.udpsocket, CLIENT_PORT))
    {
        Con_printf("Failed to initialise networked client, could not open socket\n");
        return false;
    }

    Con_printf("[CLIENT]: Client initialised\n");
    return true;
}

void CL_Shutdown( void );

void CL_Frame( void )
{
    IN_Update();
    CL_SendCmd();

    PM_Move();
    VectorCopy( pmove.origin, r_pos );

    R_Start( cl.viewangles );

    R_DrawLeaves();

    R_Present();

}



void CL_SendCmd( void )
{
    usercmd_t cmd;
    CL_MoveCmd (&cmd);
    pmove.cmd = cmd;
    
    MSG_WriteFloat (cmd.angles[0]);
    MSG_WriteFloat (cmd.angles[2]);
    MSG_WriteFloat (cmd.angles[1]);

    MSG_WriteFloat (cmd.forwardmove);
    MSG_WriteFloat (cmd.sidemove);
    MSG_WriteFloat (cmd.upmove);

    Netchan_SendPacket( &cl.chan, cl.udpsocket, USERCMD_SIZE, net_message.data );
}