#pragma once

#include "physics/shared.h"
#include "physics/backend/blockmanager.hpp"
#include "math/vector.h"

typedef struct collision_event_t
{
    physobjid_t a;
    physobjid_t b;

    vec3_t position;

    /*
    
    float penetration;
    float energy;
    vec3_t position; // moved here - paddding
    vec3_t normal;
    
    */
} collision_event_t;

class CPhysicsCollisionSolver
{
public:
// Returns true/false depending on the existence of a collision
    bool TestCollision( CBodyblockManager& BlockMananger, const physobjid_t& a, const physobjid_t& b, collision_event_t& event_out );
    void SolveCollision( const collision_event_t& event );
protected:


private:
    bool TestBroadphaseCollision( CBodyblockManager& BlockManager, const physobjid_t& a, const physobjid_t& b ); // no output event, we need to go deeper if there is one

};