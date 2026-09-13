#pragma once

#include "network/net.h"
#include "network/netchan.h"
#include "network/protocol.h"
#include "engine/player/player.h"
#include "engine/shared/cvar.h"

#define SV_PLAYER_HISTORY 4
#define SV_MAX_PLAYERS 64

typedef enum { CS_FREE, CS_CONNECTED, CS_ACTIVE } sv_client_state_t;

typedef struct {
  netchan_t chan;
  netaddr_t addr; // remote UDP addr — used to match incoming packets to this
                   // slot until netchan_t owns that lookup
  sv_client_state_t client_state;
  usercmd_t cmds[SV_PLAYER_HISTORY];
  playerstate_t history[SV_PLAYER_HISTORY];
  uint32_t last_cmd_tick;

} svplayer_t;

typedef struct {
  uint8_t initialised;

  svplayer_t clients[SV_MAX_PLAYERS];
  uint16_t numclients;

  netsocket_t sock_udp;
  netsocket_t sock_tcp;
  netaddr_t localaddr;

} server_t;

// Cvars
extern float sv_tickrate;

extern float sv_accelerate;
extern float sv_airaccelerate;
extern float sv_stopspeed;
extern float sv_maxspeed;
extern float sv_friction;
extern float sv_gravity;
extern float sv_maxplayers;

extern server_t sv_main;

void sv_updateclients(void);

// Caller must set sv_main.localaddr before calling sv_init().
void sv_think(void);
uint8_t sv_init(void);
