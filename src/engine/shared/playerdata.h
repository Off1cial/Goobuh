#pragma once

#include <stdint.h>

#include "math/vector.h"




// Data sent to the server
struct playercmd_t{
  int8_t forward, side, up;
  uint8_t movflags;
  qangle angles;
  uint8_t buttons;
};
