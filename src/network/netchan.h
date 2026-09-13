#pragma once

#include "network/net.h"

typedef struct{
  netaddr_t addr;
  uint16_t qport; // Used to differentiate clients on the same public IP
    
} netchan_t;
