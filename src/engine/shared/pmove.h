#pragma once


#include "math/vector.h"

struct playercmd_t;

void pm_accelerate(vec3_t velocity, vec3_t wish_dir, float wish_speed, float dt);
void pm_friction(vec3_t velocity, float dt);


void pm_move(struct playercmd_t* cmd, vec3_t origin, vec3_t velocity, float dt);

