
#include "common/common.h"
#include "physics/physmanager.hpp"
#include "physics/physobject.hpp"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "math/vector.h"

CPhysicsManager* g_PhysicsManager = new CPhysicsManager();



physobjid_t CPhysicsManager::CreatePhysicsObject( 
    const vec3_t origin,
    const vec3_t velocity,
    const vec3_t halfs,
    const float mass
)
{
    blockid_t block = FindBlock();
    if (block == INVALID_BLOCK_ID) return INVALID_PHYSOBJ_ID; // Failed to find or create a block

    physobjid_t physobj_id = AddBodyToBlock(block, origin, velocity, mass, halfs);
    if (physobj_id == INVALID_PHYSOBJ_ID) return INVALID_PHYSOBJ_ID; // Failed to add body to block

    return physobj_id;
}


void CPhysicsManager::SimulateBody( const physobjid_t& obj, float delta_time )
{
    blockid_t block_id = (blockid_t)((obj & PHYSOBJ_BLOCK_MASK) >> 16);
    bodyid_t body_id = (bodyid_t)(obj & PHYSOBJ_BODY_MASK);

    if (block_id < 0 || block_id >= (blockid_t)m_block_count) return; // Invalid block ID
    CBodyblock& block = m_blocks[block_id];

    if (body_id < 0 || body_id >= (bodyid_t)block.GetBodyCount()) return; // Invalid body ID

    // Update position based on velocity and delta_time
    block.x[body_id] += block.vx[body_id] * delta_time;
    block.y[body_id] += block.vy[body_id] * delta_time;
    block.z[body_id] += block.vz[body_id] * delta_time;
    #ifdef PHYS_PRINTS
    printf("Simulating body %d in block %d: New position = (%f, %f, %f)\n", body_id, block_id, block.x[body_id], block.y[body_id], block.z[body_id]);
    #endif

    // Apply gravity if enabled
    // This is a placeholder; actual gravity application would depend on the physics system's design
    block.vy[body_id] -= PHYS_DEFAULT_GRAVITY * delta_time; // Simple gravity effect
}

void CPhysicsManager::Simulate( float delta_time )
{
    // Placeholder for physics simulation logic
    // This would typically involve updating positions and velocities of all bodies based on forces, collisions, etc.
    for (blockid_t block_index = 0; block_index < m_block_count; ++block_index)
    {
        CBodyblock& block = m_blocks[block_index];
        for (bodyid_t body_index = 0; body_index < block.GetBodyCount(); ++body_index)
        {
            physobjid_t obj_id = (physobjid_t)((block_index << 16) | body_index);
            SimulateBody(obj_id, delta_time);
        }
    }
    for (blockid_t block_index = 0; block_index < m_block_count; ++block_index)
    {
        CBodyblock& block = m_blocks[block_index];
        bodyid_t block_size = block.GetBodyCount();
        for (bodyid_t body_index = 0; body_index < block_size; ++body_index)
        {
            physobjid_t obj_id = (physobjid_t)((block_index << 16) | body_index);
            for (bodyid_t other_body = 0; other_body < block_size; ++other_body ){
                if (body_index == other_body) continue;
                collision_event_t coll_event;
                TestCollision( *this, obj_id, other_body, coll_event );
            }
        }
    }


}

void CPhysicsManager::GetObjectPosition( const physobjid_t& obj, vec3_t out )
{
    blockid_t block_id = (blockid_t)((obj & PHYSOBJ_BLOCK_MASK) >> 16);
    bodyid_t body_id = (bodyid_t)(obj & PHYSOBJ_BODY_MASK);

    if (block_id < 0 || block_id >= (blockid_t)m_block_count) return; // Invalid block ID
    CBodyblock& block = m_blocks[block_id];

    if (body_id < 0 || body_id >= (bodyid_t)block.GetBodyCount()) return; // Invalid body ID

    out[0] = block.x[body_id];
    out[1] = block.y[body_id];
    out[2] = block.z[body_id];
}
