#ifndef PMOVECL_H
#define PMOVECL_H

#include "math/vector.h"
#include "engine/net/protocol.h"


typedef struct playermove_t
{
    vec3_t origin;
    vec3_t velocity;
    vec3_t viewangles;

    usercmd_t cmd;

} playermove_t;


typedef struct 
{
    float gravity;
    float stopspeed;
    float maxspeed;
    float friction;

    float accelerate;
    float airaccelerate;
} movevars_t;

extern playermove_t pmove;
extern movevars_t   movevars;
extern int onground;

void PM_Move(void);

void PM_Accelerate( vec3_t wishdir, float wishspeed, float accel );
void PM_AirAccelerate( vec3_t wishdir, float wishspeed, float accel );


#endif
