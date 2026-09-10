#pragma once
#include "math/vector.h"

// General engine-level player type, used/expanded upon by client and server

#define PM_JUMP   1
#define PM_CROUCH 2
#define PM_FIRE1  4
#define PM_FIRE2  8



typedef struct playerstate_t
{
  vec3_t origin;
  vec3_t velocity;
  qangle viewangles;
} playerstate_t;


// CLIENT: Received from the server, can be determined by cvars if client == admin
// SERVER: Determined by cvars
typedef struct pmovevars_t
{
  float dt;

  float maxspeed;
  float friction;
  float stopspeed;
  float jumpvel;
  float gravity;
  float accelerate;
  float airaccelerate;
} pmovevars_t;


typedef struct {
  int8_t mv_forward;
  int8_t mv_side;
  int8_t mv_up;
  int8_t on_ground;
  uint8_t buttons;
  uint32_t tick;
  qangle viewangles;
} usercmd_t;


void pm_move(playerstate_t* state, usercmd_t* cmd, pmovevars_t* vars);
