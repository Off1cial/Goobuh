#pragma once

#include "physics/shared.h"
#include "math/vector.h"
#include <vector>
// Optimisation Ideas:
// - Move non-gravity affected objects to a separate list? DOD

#define BODYSTATE_FLAG_FREE 0x01 // This slot in the block was freed and can be reused
#define BODYSTATE_FLAG_GRAVITY_DISABLED 0x02 // This body is not affected by gravity
#define BODYSTATE_FLAG_COLLISION_DISABLED 0x04 // This body does not collide with other bodies
#define BODYSTATE_FLAG_SLEEP 0x08 // This body is asleep and does not need to be simulated until woken up


struct aabb_t
{
    vec3_t origin, halfs;
};




// An allocated block of memory that contains several bodies and their data
class CBodyblock
{
public:
    CBodyblock( void ) { Clear(); }
    ~CBodyblock( void ) = default;

    bool Init( void );
    void Clear( void );
    void Reset( void );

    blockid_t GetBodyCount( void ) const { return m_body_count; }
    bool HasSpace( void ) const { return m_body_count < BODY_COUNT; }
    blockid_t m_body_count = 0; // Number of bodies currently in this block

    blockid_t m_block_id = INVALID_BLOCK_ID; // The ID of this block

    bodyid_t AddBody( const vec3_t position, const vec3_t velocity, const float mass, const vec3_t halfs );
    void RemoveBody( bodyid_t body_id ); // Mark the spot as free, wont be simulated

    void GetPos( bodyid_t body_id, vec3_t out );
    void SetPos( bodyid_t body_id, vec3_t pos );

    void GetVelocity( bodyid_t body_id, vec3_t out );
    void SetVelocity( bodyid_t body_id, vec3_t velocity );

    void GetHalfs( bodyid_t body_id, vec3_t out );
    void SetHalfs( bodyid_t body_id, vec3_t halfs );

    void GetAABB( bodyid_t body_id, aabb_t& out );

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
public:
    bool GetHalfs( physobjid_t obj, vec3_t out );
    bool GetPosition( physobjid_t obj, vec3_t out );

    void GetAABB( physobjid_t obj, aabb_t& out );

    std::vector<CBodyblock> m_blocks; // devious, direct access by the collision tester
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

    
    blockid_t m_block_count;
};