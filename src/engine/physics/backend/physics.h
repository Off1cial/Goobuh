#pragma once

#include <stdint.h>
#include "math/vector.h"

#define PHYS_BODYBLOCK_SIZE 64



#ifdef PHYS_DOUBLE_PRECISION
typedef double Float;
typedef double vec3_t[3];
#else
typedef float Float;
typedef float vec3_t[3];
#endif


typedef enum collidertype_t{
  COL_PLANE,
  COL_SPHERE,
  COL_BOX,
} collidertype_t;

typedef enum bodytype_t{
  BODY_BOX,
  BODY_SPHERE,
} bodytype_t;

typedef struct collider_t{
  collidertype_t type;

  union data{
    struct { Float radius; } sphere;
    struct { vec3_t extents; } box;
    struct { vec3_t normal; Float d; } plane;
  } data;

} collider_t;

typedef struct transform_t{
  vec3_t origin;
  quat_t rotation;
  vec3_t scale;
} transform_t;

// Do not change this order
struct bodyblock_t{  
  void* base;
  Float* x;
  Float* y;
  Float* z;

  Float* vx;
  Float* vy;
  Float* vz;

  Float* fx;
  Float* fy;
  Float* fz;

  collider_t* colliders;
  uint32_t count;
};


struct physworld_t{
  struct bodyblock_t* blocks;
  uint32_t numblocks;
  uint32_t capblocks;
  uint32_t numbodies;
  uint32_t capbodies;

} physworld_t;



typedef uint32_t bodyid_t;


bodyid_t _phys_create_bodybox(vec3_t origin, vec3_t half_extents);
bodyid_t _phys_create_bodysphere(vec3_t origin, Float radius);
