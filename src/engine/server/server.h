#ifndef SERVER_H
#define SERVER_H

#include "engine/net/net.h"

typedef enum
{
  SS_DEAD,
  SS_ACTIVE,
} serverstate_t;


typedef struct serverinfo_t
{
  char mapname[ 64 ];
  int numplayers;
  int maxplayers;
  u32 uptime;
} serverinfo_t;



typedef struct server_t
{
  netsock_t udpsocket;
  serverstate_t state;

  struct client_t* clients;
  int numclients;
  int maxclients;
  
  

} server_t;

bool SV_InitGame( void );
void SV_ShutdownGame( void );

#endif
