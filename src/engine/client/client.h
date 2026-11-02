#ifndef CLIENT_H
#define CLIENT_H


#include "math/vector.h"
#include "engine/net/net.h"
#include "engine/net/protocol.h"

#include "engine/shared/cvar.h"

#define CL_ARENA_SIZE 1024 * 1024
#define CLIENT_PORT 27005

bool CL_Init( void );
void CL_Shutdown( void );
void CL_Frame( void );
void CL_Think( void );

void CL_MoveCmd( usercmd_t *cmd );
void CL_SendCmd( void );

typedef struct clientstate_t
{
    netchan_t chan;
    netsock_t udpsocket;
    vec3_t viewangles;

    long servertime;
} clientstate_t;

extern clientstate_t cl;



extern cvar_t cl_maxspeed;


// Communication

#endif
