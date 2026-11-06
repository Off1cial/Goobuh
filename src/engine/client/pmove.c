#include "engine/client/pmove.h"
#include "math/vector.h"

#include "engine/map/bsp.h"


playermove_t pmove = {
    .origin = {1000, 250, 200 }    


};
movevars_t   movevars = {.accelerate = 10, .airaccelerate = 5, .friction = 10.0f, .stopspeed = 20.0f, .maxspeed = 40.0f, .gravity = 20.0f };
static vec3_t phalfs = {12, 36, 12};
int onground;

vec3_t right, up, forward;

float frametime = 1.0f / 60.0f;


static inline float HorizDistSq( const vec3_t a, const vec3_t b )
{
    float dx = a[0] - b[0];
    float dz = a[2] - b[2];
    return dx * dx + dz * dz;
}

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
#define STEPSIZE 5.0F
#define MIN_STEP_NORMAL 0.7F
#define MIN_SPEED 0.1

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
                //pmove.velocity[0] = pmove.velocity[1] = pmove.velocity[2] = 0;
                return;
            }
        }
    }
}


// Traces and also gives you the end position without needing tr.endpos
static void PM_TraceMove( const vec3_t start, const vec3_t end, trace_t *tr, vec3_t endpos )
{
    TraceBox( start, end, phalfs, tr );
    for (int i = 0; i < 3; i++)
        endpos[i] = start[i] + tr->fraction * (end[i] - start[i]);
}

static void PM_StepDown( void )
{
    vec3_t end, landed;
    trace_t tr;

    VectorCopy( pmove.origin, end );
    end[1] -= STEPSIZE;
    PM_TraceMove( pmove.origin, end, &tr, landed );

    if (tr.allsolid || tr.fraction == 1.0f)
        return;                         // nothing below, let gravity handle it
    if (tr.normal[1] < MIN_STEP_NORMAL)
        return;                         // too steep to stand on

    VectorCopy( landed, pmove.origin );
}


static void PM_StepSlideMove( void )
{
    vec3_t start_o, start_v;
    vec3_t down_o, down_v;
    vec3_t up, end, landed;
    trace_t tr;

    VectorCopy( pmove.origin,   start_o );
    VectorCopy( pmove.velocity, start_v );

    PM_SlideMove();
    VectorCopy( pmove.origin,   down_o );
    VectorCopy( pmove.velocity, down_v );

    VectorCopy( start_o, end );
    end[1] += STEPSIZE;
    PM_TraceMove( start_o, end, &tr, up );
    if (tr.allsolid)
        return;                         // keep the normal result

    float rise = up[1] - start_o[1];
    if (rise <= 0)
        return;                         // no room to go up at all

    VectorCopy( up,      pmove.origin );
    VectorCopy( start_v, pmove.velocity );
    PM_SlideMove();

    VectorCopy( pmove.origin, end );
    end[1] -= rise;
    PM_TraceMove( pmove.origin, end, &tr, landed );

    if (tr.allsolid)
        goto use_down;

    if (tr.fraction == 1.0f || tr.normal[1] < MIN_STEP_NORMAL)
        goto use_down;

    VectorCopy( landed, pmove.origin );

    if (HorizDistSq( down_o, start_o ) >= HorizDistSq( pmove.origin, start_o ))
        goto use_down;

    pmove.velocity[1] = down_v[1];
    return;

use_down:
    VectorCopy( down_o, pmove.origin );
    VectorCopy( down_v, pmove.velocity );
}



void PM_CategorisePos( void )
{
    vec3_t testpoint;
    trace_t ground_trace;

    testpoint[0] = pmove.origin[0];
    testpoint[1] = pmove.origin[1] - 1;
    testpoint[2] = pmove.origin[2];

    

    TraceBox( pmove.origin, testpoint, phalfs, &ground_trace );
    onground = ground_trace.fraction < 1.0f && ground_trace.normal[1] > 0.7f;
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

static void PM_Jump(void)
{
    if (!onground) return;

    // Add old buttons check, dont repeatedly jump when holding space

    pmove.velocity[1] += 40.0f;
    onground = false;
}

static void PM_GroundMove( void )
{
    vec3_t dst;
    trace_t tr;
    pmove.velocity[1] = 0;

    dst[0] = pmove.origin[0] + pmove.velocity[0] * frametime;
    dst[1] = pmove.origin[1];
    dst[2] = pmove.origin[2] + pmove.velocity[2] * frametime;

    TraceBox( pmove.origin, dst, phalfs, &tr );

    if (tr.fraction == 1.0f)
    {
        VectorCopy (dst, pmove.origin);
    }
    else
    {
        PM_StepSlideMove();
        pmove.velocity[1] = 0;
    }
    PM_StepDown();
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
    if (pmove.cmd.upmove)
        PM_Jump();

    
    if (onground)
    {
        PM_Friction();
        PM_Accelerate( wishdir, wishspeed, movevars.accelerate );
        PM_GroundMove();

    }
    else
    { 
        PM_AirAccelerate( wishdir, wishspeed, movevars.airaccelerate );
        pmove.velocity[1] -= movevars.gravity * frametime;
        PM_SlideMove();
    }



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
    pmove.velocity[1] += accelspeed * wishdir[1];
    pmove.velocity[2] += accelspeed * wishdir[2];

}


void PM_AirAccelerate( vec3_t wishdir, float wishspeed, float accel )
{
    float clipped = wishspeed;
    if (clipped > 30)
        clipped = 30;

    float current_speed = VectorDot( pmove.velocity, wishdir );
    float add_speed = clipped - current_speed;



    if (add_speed <= 0)
        return;

    float accelspeed = accel * wishspeed * frametime;
    
    if (accelspeed > add_speed)
        accelspeed = add_speed;

    pmove.velocity[0] += accelspeed * wishdir[0];
    pmove.velocity[1] += accelspeed * wishdir[1];
    pmove.velocity[2] += accelspeed * wishdir[2];
    //Con_printf(VectorString(pmove.origin));
}
