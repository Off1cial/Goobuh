#pragma once

#include "engine/player/player.h"

#define SV_PLAYER_HISTORY 4
#define SV_MAX_PLAYERS 64

typedef struct {
  playerstate_t state;
  usercmd_t cmds[SV_PLAYER_HISTORY];
  uint32_t last_cmd_tick;

} svplayer_t;


typedef struct {
  svplayer_t clients[SV_MAX_PLAYERS];
  uint16_t numclients;
  
} server_t;
