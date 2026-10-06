#pragma once
#include <stdint.h>
#include <vector>
#include "math/vector.h"

#include "physics/backend/blockmanager.hpp"
#include "physics/shared.h"
#include "physics/physcollision.hpp"

// Forward declaration
class CPhysicsObject;


// Idea, create an interface, dumb this down and make a physics manager for the client and one for the server.
// The client doesnt need to add/destory objects, sleep or wake them (other than ragdolls?) - but the server does
class CPhysicsManager : public CBodyblockManager, public CPhysicsCollisionSolver
{
public:
    CPhysicsManager( void ) = default;
    ~CPhysicsManager( void ) = default;


    void SetObjectPosition( const physobjid_t& obj, const vec3_t position );
    void GetObjectPosition( const physobjid_t& obj, vec3_t out );

    void SetObjectVelocity( const physobjid_t& obj, const vec3_t velocity );
    void GetObjectVelocity( const physobjid_t& obj, vec3_t out );

    void SetObjectMass( const physobjid_t& obj, const float mass );
    void GetObjectMass( const physobjid_t& obj, float& out );

    void SetObjectHalfExtents( const physobjid_t& obj, const vec3_t halfs );
    void GetObjectHalfExtents( const physobjid_t& obj, vec3_t out );

    void AddForceCentre( const physobjid_t& obj, const vec3_t force );
    void AddForceOffset( const physobjid_t& obj, const vec3_t force, const vec3_t offset );

    void SleepObject( const physobjid_t& obj );
    void WakeObject( const physobjid_t& obj );

    physobjid_t CreatePhysicsObject( 
        const vec3_t origin,
        const vec3_t velocity,
        const vec3_t halfs,
        const float mass
    );

    void DestroyPhysicsObject( CPhysicsObject& obj );

    void Simulate( float delta_time );

private:
    void SimulateBody( const physobjid_t& obj, float delta_time );
};


extern CPhysicsManager* g_PhysicsManager;