#include "engine/shared/pmove.h"

// Temporary - add cvars
float sv_accelerate = 40.0f;

void pm_accelerate(vec3_t velocity, vec3_t wish_dir, float wish_speed, float dt){
  float current_speed = DotProduct(velocity, wish_dir);

  float addspeed = wish_speed - current_speed;
  if (addspeed <= 0) return;
  
  float accelspeed = sv_accelerate * dt * wish_speed;
  if (accelspeed > addspeed)
    accelspeed = addspeed;

  velocity[0] += accelspeed * wish_dir[0];
  velocity[1] += accelspeed * wish_dir[1];
  velocity[2] += accelspeed * wish_dir[2];
}
