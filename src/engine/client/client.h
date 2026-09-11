#pragma once

#include "network/net.h"
#include "network/netchan.h"
#include "network/protocol.h"
#include "engine/player/player.h"


// Defintions available to just the client, the server defines its own interpretations

// What the client gets to know about the server
typedef struct {
  netaddr_t addr;
  char name[256];
  uint16_t numclients;
} serverinfo_t;

#define CL_CMD_BACKUP 16

typedef struct {
  usercmd_t cmds[CL_CMD_BACKUP];
  uint32_t latest_tick;
  uint32_t acked_tick;
} cmdbuffer_t;

typedef struct{
  constate_t state;
  uint32_t ticks_elapsed;


  netchan_t chan;
  netsocket_t socket_udp;
  netsocket_t socket_tcp;

  usercmd_t cmd;

  pmovevars_t pmvars; 
  cmdbuffer_t cmdbuffer;
} clientconn_t;



typedef struct 
{
  char name[256];
  clientconn_t conn;
} client_t;

extern client_t cl_main;

// Cvars
extern float cl_updaterate; // Updates per second
extern float cl_interp;


uint8_t cl_init(void);
void cl_think(void);
