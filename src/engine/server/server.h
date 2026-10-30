#ifndef SERVER_H
#define SERVER_H

#include "engine/net/net.h"
#include "engine/shared/cvar.h"

#define SERVER_PORT 27015

#define CHAN_TIMEOUT 15.0F

typedef enum
{
  SS_DEAD,
  SS_ACTIVE,
} serverstate_t;

typedef enum
{
    SSCL_ZOMBIE = 0,
    SSCL_CONNECTED = 1,
    SSCL_ACTIVE = 2,
    SSCL_DEAD = 3,
    SSCL_NUMSTATES, // not an enum
} svclientstate_t;

typedef struct serverinfo_t
{
  char mapname[ 64 ];
  int numplayers;
  int maxplayers;
  u32 uptime;
  netaddr_t netaddr;
} serverinfo_t;

typedef struct svclient_t
{
    svclientstate_t state;
    int is_fake; // bot player
    netchan_t chan;
    char name[ 64 ];
} svclient_t; 

typedef struct server_t
{
    char name[64];
    char mapname[64];

    svclient_t *clients;
    void (*client_func[3])( svclient_t *client, packet_t* packet );

    challenge_t challenge;  
    netsock_t udpsocket;

    int numclients;
    int maxclients;

    serverstate_t state;
} server_t;

extern server_t sv;
extern svclient_t* svcl;
extern sizebuf_t svsizebuff;

extern cvar_t sv_maxplayers;
extern cvar_t sv_cheats;


extern cvar_t sv_accelerate;
extern cvar_t sv_airaccelerate;

extern cvar_t sv_maxspeed;




// sv_main.c
bool SV_Init( char name[64], int maxplayers );
void SV_Shutdown( void );
bool SV_InitGame( void );
void SV_ShutdownGame( void );

void SV_HandleZombieClient( svclient_t* client, packet_t *latest );
void SV_HandleConnectedClient( svclient_t* client, packet_t* latest );
void SV_HandleActiveClient( svclient_t* client, packet_t* latest );

void SV_HandleUnconnected( void );


void SV_DropClient( svclient_t* client );

void* SV_Alloc( size_t size );
void SV_Mark( void );
void SV_FreeToMark( void );

#endif
