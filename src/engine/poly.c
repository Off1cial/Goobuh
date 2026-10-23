#include "engine/poly.h"
#include "math/vector.h"

#define ON_EPSILON 1e-6f

void PlaneWinding( plane_t plane, winding_t* out )
{
    if (!out) return;
    static const float size = 500.0f;
    
    vec3_t centre;
    VectorScale( plane.normal, plane.d, centre );
    
    // Find tangent
    vec3_t tangent;
    vec3_t bit_tangent;
    vec3_t help;

    if (fabsf(plane.normal[1]) > 0.9f)
    {
        VectorCopy( AXIS_Z, help );
    }
    else
    {
        VectorCopy( AXIS_Y, help );
    }


    VectorCrossNorm(plane.normal, help, tangent );
    VectorCrossNorm(plane.normal, tangent, bit_tangent);

    // Find corners
    vec3_t t_add;
    vec3_t b_add;
    VectorScale( tangent, size, t_add );
    VectorScale( bit_tangent, size, b_add );

    VectorAdd( centre, t_add, out->p[0] );
    VectorAdd( out->p[0], b_add, out->p[0]);
    
    VectorAdd( centre, t_add, out->p[1] );
    VectorSub( out->p[1], b_add, out->p[1]);
    
    VectorSub( centre, t_add, out->p[2] );
    VectorSub( centre, b_add, out->p[2] );

    VectorSub( centre, t_add, out->p[3] );
    VectorAdd( centre, b_add, out->p[3] );
    out->numpoints = 4;
}


void ClipWinding( winding_t* in, plane_t p, winding_t* out )
{
    if (!in || !out) return;
    
    winding_t win = {0};

    int point, nextpoint;

    for (point = 0; point < in->numpoints; point++)
    {
        nextpoint = (point + 1 ) % in->numpoints;
        // We have a line, point -> nextpoint
        
        // Keep whats behind the plane
        float dp = VectorDot(p.normal, in->p[point]) - p.d;
        float dpn = VectorDot(p.normal, in->p[nextpoint]) - p.d;

        bool p_behind = dp < ON_EPSILON;
        bool pn_behind = dpn < ON_EPSILON;
    
        if (p_behind)
        {
            VectorCopy( in->p[point], win.p[win.numpoints] );
            win.numpoints++;
        }

        if (p_behind != pn_behind)
        {
            float t = dp / (dp - dpn );
            vec3_t delta, intersection;
            VectorSub( in->p[nextpoint], in->p[point], delta );
            VectorMA( in->p[point], t, delta, win.p[win.numpoints++]);
        }

    }
    *out = win; 
}
