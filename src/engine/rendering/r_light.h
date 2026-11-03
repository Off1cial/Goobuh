#ifndef R_LIGHT_H
#define R_LIGHT_H

#include "math/vector.h"

enum { light_spot, light_env, light_point };

#define MAX_DLIGHTS 8 // Keep up to date in the shader
typedef struct light_gpu_t
{
    float   position[3];  // 0
    uint8_t col[4];       // 12
    float   direction[3]; // 16
    float   intensity;    // 28
    int32_t type;         // 32
    float   radius;       // 36
    float   cutoff;       // 40
    float   _pad;         // 44
} light_gpu_t;            // 48

_Static_assert(sizeof(light_gpu_t) == 48, "std140 stride");
_Static_assert(OFFSETOF(light_gpu_t, direction) == 16, "direction");
_Static_assert(OFFSETOF(light_gpu_t, type) == 32, "type");

typedef struct light_t
{
    int type;
    byte col[4];
    float intensity;

    union 
    {
        struct { vec3_t position; vec3_t direction; float cutoff; } spot;
        struct { vec3_t direction; } env;
        struct { vec3_t position; float radius; } point;
    };

} light_t;


#endif