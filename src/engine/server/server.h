#ifndef SERVER_H
#define SERVER_H

#include "engine/net/net.h"

#define SERVER_PORT 27015


typedef enum
{
  SS_DEAD,
  SS_ACTIVE,
} serverstate_t;

typedef enum
{
    SSCL_DEAD,
    SSCL_ZOMBIE,
    SSCL_CONNECTED,
    SSCL_ACTIVE,
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
  netsock_t udpsocket;
  serverstate_t state;

  svclient_t *clients;
  int numclients;
  int maxclients;
  
  

} server_t;

extern server_t sv;
// sv_main.c
bool SV_Init( char name[64], int maxplayers );
void SV_Shutdown( void );
bool SV_InitGame( void );
void SV_ShutdownGame( void );

void* SV_Alloc( size_t size );
void SV_Mark( void );
void SV_FreeToMark( void );

#endif
