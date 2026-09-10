#pragma once

#include "common/common.h"

typedef struct InputState InputState;



struct playercontroller_t
{
  u8 null_cancel; // Does pressing A and D cancel sideways movement?
  i8 forward, side, up;
  u8 movflags;

  float cam_sens;
  float cam_lookx;
  float cam_looky;
};

struct playercmd_t;

void playercontroller_update(struct playercontroller_t* cont, InputState* input);
