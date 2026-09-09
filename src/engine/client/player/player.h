#pragma once

#include "math/vector.h"

#include "renderer/camera.h"
#include "engine/client/player/playercontroller.h"



typedef struct player_t{
  struct playercontroller_t controller;

  vec3_t origin;
  vec3_t velocity;

  camera_t* camera;

} player_t;




extern player_t g_player;


void player_init(vec3_t origin, camera_t* camera);
void player_think(InputState* input, float dt);


