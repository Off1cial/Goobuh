#pragma once

#include "physics/interface/physobject.hpp"
#include "physics/shared.h"

class CPhysicsObject : public IPhysicsObject
{
public:
    ~CPhysicsObject() = default;

    void DisableGravity( void );
    void EnableGravity( float gravity = PHYS_DEFAULT_GRAVITY );

    bool IsGravityEnabled( void ) const { return m_gravity_enabled; }

    physobjid_t m_ref = INVALID_PHYSOBJ_ID;
private:
    bool m_gravity_enabled = true; // fabsf(m_gravity) > 0.0f
    float m_gravity = PHYS_DEFAULT_GRAVITY; // Default gravity value
};


CPhysicsObject CreatePhysicsObject( 
    const vec3_t& origin,
    const vec3_t& velocity,
    const vec3_t& halfs,
    const float mass
);

void DestroyPhysicsObject( CPhysicsObject& obj );