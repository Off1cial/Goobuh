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
  

} server_t;

// Cvars
extern float sv_accelerate;
extern float sv_airaccelerate;
extern float sv_stopspeed;
extern float sv_maxspeed;
extern float sv_friction;
extern float sv_gravity;
extern float sv_maxplayers;


extern server_t sv_main;
