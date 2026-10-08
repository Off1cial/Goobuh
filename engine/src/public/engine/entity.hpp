#pragma once

#include "math/vector.h"

typedef struct entity_s entity_t;
typedef struct AssetHandle AssetHandle;


class IEntity
{
protected:
    virtual bool Spawn( vec3_t origin, vec3_t velocity, vec3_t angles, float mass ) = 0;
    virtual void EnablePhysics( float mass ) = 0;
    virtual void DisablePhysics( void ) = 0;
    virtual void Destroy( void ) = 0;

    virtual void OnCollision( void ) = 0; // Not called by physics, physics sets a flag in the body, the entity reads it and calls this afterwards
};


