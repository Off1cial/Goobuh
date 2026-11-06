#include "engine/client/client.h"

#include "engine/client/cl_input.h"
#include "engine/client/pmove.h"
#include "engine/rendering/renderer.h"
#include "engine/gui/gui.h"
#include "engine/net/net.h"
#include "engine/net/msg.h"
#include "engine/map/bsp.h"

clientstate_t cl = {0};
sysarena_t cl_arena = {0};


cvar_t cl_maxspeed = {.name = "cl_maxspeed", .string = "40.0f", .value = 40.0f, .next = NULL};

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

    Netchan_Setup (&cl.chan);
    Con_printf("[CLIENT]: Client initialised\n");
    return true;
}

void CL_Shutdown( void );

void CL_Frame( void )
{
    IN_Update();
    R_Start( cl.viewangles );

    CL_SendCmd();

    PM_Move();
    VectorCopy( pmove.origin, r_pos );
    r_pos[1] += 36;


    R_DrawLeaves();

    GUI_FrameStart();
    
    GUI_DrawRect(200, 40, 80, 200, 128, 255, 255, 100);

    GUI_FrameEnd();


    R_Present();

}



void CL_SendCmd( void )
{
    usercmd_t cmd;
    CL_MoveCmd (&cmd);
    pmove.cmd = cmd;
    
    union 
    {
        usercmd_t cmd;
        byte b[];
    } data;
    data.cmd = cmd;

    Netchan_SendPacket( &cl.chan, cl.udpsocket, USERCMD_SIZE, data.b);
}


void CL_Disconnect( void )
{
    cl.state = cl_free;
    cl.servertime = 0;
}

void CL_RequestChallenge( void )
{
    MSG_WriteByte (PKT_CHALLENGE_REQUEST);
    MSG_WriteShort(cl.chan.qport);
    MSG_WriteLong (3);
    Netchan_SendPacket( &cl.chan, cl.udpsocket, 3, net_message.data );
    cl.state = cl_waiting;
}

void CL_BeginConnect( const char* server )
{
    CL_Disconnect();
    CL_RequestChallenge();
}

void CL_ReadPackets( void )
{
    while (NET_GetPacket())
    {
        if (!Netchan_Process( &cl.chan ))
            continue;

        short qport;
        MSG_ReadShort(0, &qport);
        if (qport != cl.chan.qport)
            continue;
        

    }
}
