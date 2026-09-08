#include "engine/client/player/player.h"


#include "math/vector.h"

player_t g_player = {0};


void player_init(vec3_t origin, camera_t* camera){
  g_player.camera = camera;

  memset(&g_player.controller, 0, sizeof(struct playercontroller_t));

  VectorCopy(origin, g_player.origin);
  VectorSet(g_player.velocity, 0, 0, 0);
}
