#ifndef PLANE_H
#define PLANE_H

#include "math/vector.h"

typedef struct plane_t
{
    vec3_t normal;
    float d;
} plane_t;

FORCEINLINE void PlaneReverse( plane_t p, plane_t* out )
{
    out->d = -p.d;
    VectorScale( p.normal, -1, out->normal );
}

FORCEINLINE bool PlaneEqual( plane_t a, plane_t b )
{
#ifndef EPSILON
#define EPSILON 1e-6f
#endif
    if (fabsf(b.d - a.d) > EPSILON) return false;
    bool q = 
        (fabsf(a.normal[0] - b.normal[0]) > EPSILON) ||
        (fabsf(a.normal[1] - b.normal[1]) > EPSILON) ||
        (fabsf(a.normal[2] - b.normal[2]) > EPSILON);
    return !q;
}


#endif
