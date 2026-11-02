#include "engine/client/pmove.h"
#include "math/vector.h"



playermove_t pmove = {0};
movevars_t   movevars;
int onground;

vec3_t right, up, forward;

float frametime = 1.0f / 60.0f;

void PM_Move( void )
{
    vec3_t wishdir;
    float wishspeed;

    VectorCopy( pmove.cmd.angles, pmove.viewangles );
    AngleBasis( pmove.viewangles, forward, right, up );

    wishdir[0] = forward[0] * pmove.cmd.forwardmove + right[0] * pmove.cmd.sidemove;
    wishdir[1] = pmove.cmd.upmove;
    wishdir[2] = forward[2] * pmove.cmd.forwardmove + right[2] * pmove.cmd.sidemove;

    Con_printf(VectorString(wishdir));

    wishspeed = VectorNormalise( wishdir );

    PM_Accelerate( wishdir, wishspeed, 20 );




    pmove.origin[0] += pmove.velocity[0] * frametime;
    pmove.origin[1] += pmove.velocity[1] * frametime;
    pmove.origin[2] += pmove.velocity[2] * frametime;
    //Con_printf(VectorString(pmove.origin));
}


void PM_Accelerate( vec3_t wishdir, float wishspeed, float accel )
{
    float current_speed = VectorDot( pmove.velocity, wishdir );
    float add_speed = wishspeed - current_speed;

    Con_printf("Add speed = %0.2f\n", add_speed );

    if (add_speed <= 0)
        return;

    float accelspeed = accel * wishspeed * frametime;
    
    if (accelspeed > add_speed)
        accelspeed = add_speed;

    Con_printf("wishspeed = %g\n", wishspeed );
    Con_printf("Accel speed = %g\n", accelspeed) ;
    pmove.velocity[0] += accelspeed * wishdir[0];
    pmove.velocity[1] += accelspeed * wishdir[1];
    pmove.velocity[2] += accelspeed * wishdir[2];
    //Con_printf(VectorString(pmove.origin));
}
