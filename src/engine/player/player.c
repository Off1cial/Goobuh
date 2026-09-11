#include "engine/player/player.h"
#include "network/protocol.h"

#define PM_HASMASK(flag, mask) ( (flag & mask) != 0 )

void pm_friction(vec3_t velocity, float friction, float stopspeed, float dt)
{
  vec3_t vel;
  VectorCopy(velocity, vel);
  float speed = sqrtf(vel[0] * vel[0] + vel[1] * vel[1] + vel[2] * vel[2]);
  if (speed < 0.0001f)
    return;

  float control = speed < stopspeed ? stopspeed : speed;
  float drop = control * friction * dt;

  float newspeed = speed - drop;
  if (newspeed < 0)
    newspeed = 0;
  newspeed /= speed;

  vel[0] *= newspeed;
  vel[1] *= newspeed;
  vel[2] *= newspeed;
  VectorCopy(vel, velocity);
}

void pm_accelerate(vec3_t velocity, vec3_t wishdir, float wishspeed, float acceleration, float dt){
 float currentspeed = DotProduct(velocity, wishdir);
  float addspeed = wishspeed - currentspeed;
  if (addspeed <= 0)
    return;

  float accelspeed = acceleration * dt * wishspeed;
  if (accelspeed > addspeed)
    accelspeed = addspeed;

  velocity[0] += accelspeed * wishdir[0];
  velocity[1] += accelspeed * wishdir[1];
  velocity[2] += accelspeed * wishdir[2];
}


// Temporary defintion, this might want its own wishvel and wishdir
void pm_airaccelerate(vec3_t velocity, vec3_t wishdir, float wishspeed, float acceleration, float dt);

void pm_move(playerstate_t* state, usercmd_t* cmd, pmovevars_t* vars){
  pm_friction(state->velocity, vars->friction, vars->stopspeed, vars->dt);

  vec3_t front, right;
  AnglesVector(state->viewangles, front);
  VectorCrossNorm(front, AXIS_Y, right);

  vec3_t wishvel, wishdir;
  wishvel[0] = vars->maxspeed * (front[0] * cmd->mv_forward + right[0] * cmd->mv_side); 
  wishvel[1] = 0;
  wishvel[2] = vars->maxspeed * (front[2] * cmd->mv_forward + right[2] * cmd->mv_side);

  VectorCopy(wishvel, wishdir);
  float wishspeed = VectorNormalise(wishdir);
  if (wishspeed > vars->maxspeed)
    wishspeed = vars->maxspeed;
  pm_accelerate(state->velocity, wishdir, wishspeed, vars->accelerate, vars->dt);
}
