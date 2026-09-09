#include "engine/client/player/player.h"
#include "engine/shared/playerdata.h"
#include "engine/shared/pmove.h"

#include "math/vector.h"

player_t g_player = {0};


void player_init(vec3_t origin, camera_t* camera){
  g_player.camera = camera;

  memset(&g_player.controller, 0, sizeof(struct playercontroller_t));

  VectorCopy(origin, g_player.origin);
  VectorSet(g_player.velocity, 0, 0, 0);
}


void player_think(InputState* input, float dt){
  struct playercmd_t cmd_out;
  // update controller -> look around -> move -> make command
  playercontroller_update(&g_player.controller, input);
  camera_look(
      g_player.camera, 
      g_player.controller.cam_lookx, 
      g_player.controller.cam_looky, 
      g_player.controller.cam_sens
  );
  camera_update(g_player.camera);



  VectorCopy(g_player.camera->angles, cmd_out.angles);
  cmd_out.movflags = g_player.controller.movflags;
  cmd_out.forward = g_player.controller.forward;
  cmd_out.side = g_player.controller.side;
  cmd_out.up = g_player.controller.up;

  pm_move(&cmd_out, g_player.origin, g_player.velocity, dt);
  VectorCopy(g_player.origin, g_player.camera->origin);
}
