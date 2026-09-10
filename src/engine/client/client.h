#pragma once

#include "network/net.h"


// What the client gets to know about the server
typedef struct {
  netaddr_t addr;
  char name[256];
  uint16_t numclients;
} serverinfo_t;





typedef struct{
  constate_t state;


  uint32_t ticks_elapsed;
  netaddr_t server_addr;
} clientconn_t;



typedef struct 
{


  clientconn_t conn;
} client_t;
