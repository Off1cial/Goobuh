#pragma once

#include "math/vector.h"

#define PHYSBOX_INVALID (-INT32_MAX)

using physbox_t = int32_t;


struct bodyblock_t
{
    void* allocation;
    
    float* x, *y, *z;
    float* vx, *vy, *vz;
};



class CPhysManager
{
public:
    CPhysManager() = default;
    ~CPhysManager() = default;

    physbox_t CreatePhysbox( vec3_t origin, vec3_t velocity, vec3_t halfs, float mass ); // No angles yet, all AABB for now - HL1 style


};