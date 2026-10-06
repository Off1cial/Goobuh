#include "physics/physobject.hpp"
#include "physics/physmanager.hpp"



CPhysicsObject CreatePhysicsObject( 
    const vec3_t& origin,
    const vec3_t& velocity,
    const vec3_t& halfs,
    const float mass
)
{
    CPhysicsObject obj;
    physobjid_t id = g_PhysicsManager->CreatePhysicsObject( origin, velocity, halfs, mass );
    obj.m_ref= id;
    return obj;
}