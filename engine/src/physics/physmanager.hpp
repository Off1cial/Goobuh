#pragma once
#include <stdint.h>
#include <vector>
#include "math/vector.h"

#include "physics/shared.h"

// Optimisation Ideas:
// - Move non-gravity affected objects to a separate list? DOD

#define BODYSTATE_FLAG_FREE 0x01 // This slot in the block was freed and can be reused
#define BODYSTATE_FLAG_GRAVITY_DISABLED 0x02 // This body is not affected by gravity
#define BODYSTATE_FLAG_COLLISION_DISABLED 0x04 // This body does not collide with other bodies
#define BODYSTATE_FLAG_SLEEP 0x08 // This body is asleep and does not need to be simulated until woken up

// Forward declaration
class CPhysicsObject;

// An allocated block of memory that contains several bodies and their data
class CBodyblock
{
public:
    CBodyblock( void ) { Clear(); }
    ~CBodyblock( void ) = default;

    bool Init( void );
    void Clear( void );
    void Reset( void );

    size_t GetBodyCount( void ) const { return m_body_count; }
    bool HasSpace( void ) const { return m_body_count < BODY_COUNT; }
    size_t m_body_count = 0; // Number of bodies currently in this block

    blockid_t m_block_id = INVALID_BLOCK_ID; // The ID of this block

    bodyid_t AddBody( const vec3_t position, const vec3_t velocity, const float mass, const vec3_t halfs );
    void RemoveBody( bodyid_t body_id ); // Mark the spot as free, wont be simulated

    void SleepBody( bodyid_t body_id );
    void WakeBody( bodyid_t body_id );

    constexpr static size_t BODY_COUNT = 64; // Number of bodies per block
    constexpr static size_t BODY_SIZE = sizeof(float) * 10 + sizeof(char); // Size of each body in bytes (3 for position, 3 for velocity, 1 for mass, 3 for half extents, 1 for state flags)
    constexpr static size_t ALLOCATION_SIZE = BODY_COUNT * BODY_SIZE; // Total size of the allocation in bytes



    void* allocation = nullptr; // The allocated memory for this block
    // Offsets into the allocation for each field
    char* state_flags;
    float* x, *y, *z; // Position data
    float *vx, *vy, *vz; // Velocity data
    float *mass; // Mass data
    float *hx, *hy, *hz; // Half extents data


};

class CBodyblockManager
{
protected:
    CBodyblockManager( void ) = default;
    ~CBodyblockManager( void ) = default;

    void Shutdown( void );

    blockid_t CreateBlock( void );
    blockid_t FindBlock( void ); // Find a block with space for a new body, or create a new one if none exist

    physobjid_t AddBodyToBlock( blockid_t block, const vec3_t position, const vec3_t velocity, const float mass, const vec3_t halfs );
    void RemoveBodyFromBlock( physobjid_t obj );

    void SleepBodyInBlock( physobjid_t obj );
    void WakeBodyInBlock( physobjid_t obj );

    std::vector<CBodyblock> m_blocks;
    size_t m_block_count;
};

class CPhysicsManager : public CBodyblockManager
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