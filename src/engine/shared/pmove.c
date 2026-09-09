#include "engine/shared/pmove.h"
#include "engine/shared/cvar.h"
#include "engine/shared/playerdata.h"

#include <stdio.h>

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


void pm_friction(vec3_t velocity, float dt)
{
  vec3_t vel;
  VectorCopy(velocity, vel);
  float speed = sqrtf(vel[0] * vel[0] + vel[1] * vel[1] + vel[2] * vel[2]);
  if (speed < 0.0001f)
    return;

  float control = speed < sv_stopspeed ? sv_stopspeed : speed;
  float drop = control * sv_friction * dt;

  float newspeed = speed - drop;
  if (newspeed < 0)
    newspeed = 0;
  newspeed /= speed;

  vel[0] *= newspeed;
  vel[1] *= newspeed;
  vel[2] *= newspeed;
  VectorCopy(vel, velocity);
}

void pm_move(struct playercmd_t* cmd, vec3_t origin, vec3_t velocity, float dt){
  pm_friction(velocity, dt);
   
  vec3_t forward, right;
  AnglesVector(cmd->angles, forward);
  VectorCrossNorm(forward, AXIS_Y, right);

  vec3_t wishvel, wishdir;

  wishvel[0] = sv_maxspeed * (forward[0] * cmd->forward + right[0] * cmd->side);
  wishvel[1] = 0; // 0 for now - no vertical movement
  wishvel[2] = sv_maxspeed * (forward[2] * cmd->forward + right[2] * cmd->side);

  //printf("Wishvel = (%0.2f, %0.2f, %0.2f)\n", wishvel[0], wishvel[1], wishvel[2]);
  VectorCopy(wishvel, wishdir);
  float wishspeed = VectorNormalise(wishdir);
  if (wishspeed > sv_maxspeed)
    wishspeed = sv_maxspeed;

  pm_accelerate(velocity, wishdir, wishspeed, dt);
  VectorMA(origin, dt, velocity, origin);
}
