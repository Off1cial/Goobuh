#include "engine/server/server.h"
#include "engine/player/player.h"

usercmd_t _lastest_cmd(usercmd_t* cmds, size_t n){
  uint32_t latest_tick = 0;
  usercmd_t latestcmd = {0};
  for (size_t i = 0; i < n; i++){
    uint32_t tick = cmds[i].tick;
    if (tick > latest_tick){
      latest_tick = tick;
      latestcmd = cmds[i];
    }
  }
  return latestcmd;
}

void sv_updateclients(void){
  for (uint16_t i = 0; i < sv_main.numclients; i++){
    
  }
}