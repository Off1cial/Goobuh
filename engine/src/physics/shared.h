#pragma once

#include <stdint.h>
#include "common/common.h"

typedef int16_t blockid_t;
typedef int16_t bodyid_t;
typedef int32_t physobjid_t;


// Full physobj_t = blockid_t | bodyid_t


#define INVALID_BLOCK_ID -1
#define INVALID_BODY_ID INVALID_BLOCK_ID
#define INVALID_PHYSOBJ_ID -1
#define PHYSOBJ_BLOCK_MASK 0xFFFF0000
#define PHYSOBJ_BODY_MASK 0x0000FFFF

#define PHYS_DEFAULT_GRAVITY 9.81f

//#define PHYS_PRINTS

#define KINETIC_ENERGY( mass, speed ) (0.5f * mass * speed * speed) //  (1/2) mv^2


#define OBJ_ID_SEPARATE( obj, bodyout, blockout ) \
    blockout = (blockid_t)(obj >> 16);\
    bodyout = (bodyid_t)(obj && PHYSOBJ_BODY_MASK)



static FORCEINLINE physobjid_t physobjid( blockid_t block, bodyid_t body ){
  return (physobjid_t)( ((physobjid_t)block << 16) | body );
}