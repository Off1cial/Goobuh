#ifndef POLYGON_H
#define POLYGON_H

#include "math/vector.h"
#include "math/plane.h"
#include "common/common.h"

#define MAX_WINDING_POINTS 64

typedef struct winding_t
{
    vec3_t p[MAX_WINDING_POINTS];
    int numpoints;
} winding_t;

static FORCEINLINE bool WINDING_VALID( winding_t win )
{
    return win.numpoints >= 3;
}


void PlaneWinding( plane_t p, winding_t* out );

// Keep whats behind the plane
void ClipWinding( winding_t* in, plane_t p, winding_t* out );


#endif
