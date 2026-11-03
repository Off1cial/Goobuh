#include "engine/client/pmove.h"
#include "math/vector.h"

#include "engine/map/bsp.h"


playermove_t pmove = {
    .origin = {400, 0, 5 }    


};
movevars_t   movevars = {.accelerate = 20, .airaccelerate = 10, .friction = 10.0f, .stopspeed = 20.0f, .maxspeed = 40.0f, .gravity = 9.80f };
static vec3_t phalfs = {12, 36, 12};
int onground;

vec3_t right, up, forward;

float frametime = 1.0f / 60.0f;


static int PM_ClipVelocity( const vec3_t in, const vec3_t normal, vec3_t out, float overbounce )
{

    int blocked = 0;
    if (normal[1] > 0)
        blocked |= 1;
    if (!normal[1])
        blocked |= 2;
    
    float backoff = VectorDot( in, normal ) * overbounce;
    for (int i = 0; i < 3; i++)
    {
        out[i] = in[i] - normal[i] * backoff;
    }
    return blocked;
}

#define MAX_CLIP_PLANES 4

static void PM_SlideMove( void )
{
    vec3_t planes[MAX_CLIP_PLANES];
    int    numplanes = 0;
    float  timeleft  = frametime;

    for (int bump = 0; bump < MAX_CLIP_PLANES; bump++)
    {
        vec3_t end;
        VectorMA( pmove.origin, timeleft, pmove.velocity, end );

        trace_t tr;
        TraceBox( pmove.origin, end, phalfs, &tr );

        if (tr.allsolid)
        {
            // Stuck inside solid: stop moving
            pmove.velocity[0] = pmove.velocity[1] = pmove.velocity[2] = 0;
            return;
        }

        // Move as far as the trace allowed
        VectorMA( pmove.origin, tr.fraction * timeleft, pmove.velocity, pmove.origin );

        if (tr.fraction == 1.0f)
            break;

        timeleft *= 1.0f - tr.fraction;
        VectorCopy( tr.normal, planes[numplanes] );
        numplanes++;

        // Remove velocity going into every plane we've touched
        int i, j;
        for (i = 0; i < numplanes; i++)
        {
            PM_ClipVelocity( pmove.velocity, planes[i], pmove.velocity, 1.001f );
            for (j = 0; j < numplanes; j++)
                if (j != i && VectorDot( pmove.velocity, planes[j] ) < 0)
                    break;
            if (j == numplanes)
                break;      // velocity is clear of all planes
        }

        if (i == numplanes)
        {
            if (numplanes == 2)
            {
                // Slide along the crease between the two planes
                vec3_t dir;
                VectorCross( planes[0], planes[1], dir );
                VectorNormalise (dir);
                VectorScale( dir, VectorDot( dir, pmove.velocity ), pmove.velocity );
            }
            else
            {
                pmove.velocity[0] = pmove.velocity[1] = pmove.velocity[2] = 0;
                return;
            }
        }
    }
}



void PM_CategorisePos( void )
{
    vec3_t testpoint;
    trace_t ground_trace;

    testpoint[0] = pmove.origin[0];
    testpoint[1] = pmove.origin[1] - 1;
    testpoint[2] = pmove.origin[2];

    

    TraceBox( pmove.origin, testpoint, phalfs, &ground_trace );
    onground = ground_trace.normal[1] > 0.7f;
    /*
    Con_printf("ground fraction: %g, normal: (%0.2f, %0.2f, %0.2f), onground: %d\n",
    ground_trace.fraction,
    ground_trace.normal[0], ground_trace.normal[1], ground_trace.normal[2]
    ,
    onground);
    */
}

void PM_Friction( void )
{
    float *vel;
    float speed, newspeed;
    float friction, control, drop;


    vel = pmove.velocity;

    speed = sqrtf( vel[0] * vel[0] + vel[2] * vel[2] );

    if ( speed < 1 )
    {
        vel[0] = 0;
        vel[2] = 0;
        return;
    }

    friction = movevars.friction;
    drop = 0;
    control = speed < movevars.stopspeed ? movevars.stopspeed : speed;
    drop += control * friction * frametime;
    
    
    newspeed = speed - drop;
    if (newspeed < 0)
        newspeed = 0;
    newspeed /= speed;

    vel[0] *= newspeed;
    vel[2] *= newspeed;
}


void PM_Move( void )
{
    vec3_t wishdir;
    float wishspeed;



    VectorCopy( pmove.cmd.angles, pmove.viewangles );
    AngleBasis( pmove.viewangles, forward, right, up );

    wishdir[0] = forward[0] * pmove.cmd.forwardmove + right[0] * pmove.cmd.sidemove;
    wishdir[1] = 0;
    wishdir[2] = forward[2] * pmove.cmd.forwardmove + right[2] * pmove.cmd.sidemove;
    wishspeed = VectorNormalise( wishdir );
    if (wishspeed > movevars.maxspeed)
        wishspeed = movevars.maxspeed;
    PM_CategorisePos();
    if (onground && pmove.velocity[1] < 0)
        pmove.velocity[1] = 0;

    if (onground && pmove.cmd.upmove) pmove.velocity[1] += 40.0f; 

    PM_Friction();
    PM_Accelerate( wishdir, wishspeed, movevars.accelerate );
    if (!onground) pmove.velocity[1] -= movevars.gravity * frametime;




    PM_SlideMove();
    //Con_printf(VectorString(pmove.origin));
    PM_CategorisePos();
}


void PM_Accelerate( vec3_t wishdir, float wishspeed, float accel )
{
    float current_speed = VectorDot( pmove.velocity, wishdir );
    float add_speed = wishspeed - current_speed;


    if (add_speed <= 0)
        return;

    float accelspeed = accel * wishspeed * frametime;
    
    if (accelspeed > add_speed)
        accelspeed = add_speed;

    pmove.velocity[0] += accelspeed * wishdir[0];
    //pmove.velocity[1] += accelspeed * wishdir[1];
    pmove.velocity[2] += accelspeed * wishdir[2];
    //Con_printf(VectorString(pmove.origin));
}
