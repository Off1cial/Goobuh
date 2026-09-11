#include "engine/server/server.h"
#include "engine/shared/cvar.h"



// Cvars - Half-Life convention
float sv_accelerate = 10.0f;
float sv_airaccelerate = 10.0f;
float sv_stopspeed = 100.0f; 
float sv_maxspeed = 320.0f;
float sv_friction = 4.0f;
float sv_gravity = 600.0f;
float sv_maxplayers = 32;



static svplayer_t* alloc_client(netaddr_t addr, const char* name){
  for (uint16_t i = 0; i < sv_maxplayers; i++){
    if (sv_main.clients[i].client_state != CS_FREE) continue; 
    
    svplayer_t* client = &sv_main.clients[i];
    memset(client, 0, sizeof(*client));
    client->chan.remote = addr;
    client->client_state = CS_CONNECTED;
    // Setup netchan
    sv_main.numclients++;
    return client;
  }
  return NULL;
}

uint8_t sv_init(void){
  memset(&sv_main, 0, sizeof(server_t));
  sv_main.sock_tcp = -1;
  sv_main.sock_udp = -1;
  
  Cvar_RegisterLinked(
      "sv_accelerate", 
      "10.0f",
      CVAR_SERVER, 
      &sv_accelerate);
  Cvar_RegisterLinked(
      "sv_airaccelerate", 
      "10.0f", 
      CVAR_SERVER, 
      &sv_airaccelerate);
  Cvar_RegisterLinked(
      "sv_stopspeed",
      "100.0f",
      CVAR_SERVER,
      &sv_stopspeed
      );
  Cvar_RegisterLinked(
      "sv_maxspeed",
      "320.0f",
      CVAR_SERVER,
      &sv_friction
      );

  Cvar_RegisterLinked(
      "sv_gravity",
      "600.0f",
      CVAR_SERVER,
      &sv_gravity
      );

  Cvar_RegisterLinked(
      "sv_maxplayers",
      "32",
      CVAR_SERVER,
      &sv_maxplayers
      );

  return 1;
}


