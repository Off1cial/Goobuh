#pragma once

#include "common/common.h"

typedef struct InputState InputState;

#define PLAYERCONT_MOV_FWD    1 << 0
#define PLAYERCONT_MOV_BACK   1 << 1
#define PLAYERCONT_MOV_RIGHT  1 << 2
#define PLAYERCONT_MOV_LEFT   1 << 3
#define PLAYERCONT_MOV_UP     1 << 4
#define PLAYERCONT_MOV_DOWN   1 << 5


struct playercontroller_t
{
  u8 null_cancel; // Does pressing A and D cancel sideways movement?
  u8 movflags;

  float cam_sens;
  float cam_lookx;
  float cam_looky;
};

void playercontroller_update(struct playercontroller_t* cont, InputState* input);
