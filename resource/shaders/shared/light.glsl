#ifndef LIGHT_SH
#define LIGHT_SH

#define MAX_DLIGHTS 8   // Change with the engine

// Must match the enum in r_light.h
#define LIGHT_SPOT  0
#define LIGHT_ENV   1
#define LIGHT_POINT 2

// Must match light_gpu_t in r_light.h (48 byte std140 stride)
struct light_t
{
    vec3  position;   // 0   spot & point
    uint  col;        // 12  packed RGBA8, fills vec3 padding
    vec3  direction;  // 16  spot & env
    float intensity;  // 28  all
    int   type;       // 32
    float radius;     // 36  point
    float cutoff;     // 40  spot (cosine of the half angle)
                      // 44  implicit padding
};
#endif