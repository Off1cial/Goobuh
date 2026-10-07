
#include "common/common.h"
#include "physics/physmanager.hpp"
#include "physics/physobject.hpp"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "math/vector.h"
#include "math/quat.h"

CPhysicsManager* g_PhysicsManager = new CPhysicsManager();



physobjid_t CPhysicsManager::CreatePhysicsObject( 
    const vec3_t origin,
    const vec3_t velocity,
    const vec3_t halfs,
    const qangle angles,
    const float mass
)
{
    blockid_t block = FindBlock();
    if (block == INVALID_BLOCK_ID) return INVALID_PHYSOBJ_ID; // Failed to find or create a block

    physobjid_t physobj_id = AddBodyToBlock(block, origin, velocity, mass, halfs, angles);
    if (physobj_id == INVALID_PHYSOBJ_ID) return INVALID_PHYSOBJ_ID; // Failed to add body to block

    return physobj_id;
}

void CPhysicsManager::DestroyPhysicsObject( physobjid_t& obj )
{
    blockid_t block_id = (blockid_t)((obj & PHYSOBJ_BLOCK_MASK) >> 16);
    bodyid_t body_id = (bodyid_t)(obj & PHYSOBJ_BODY_MASK);


}
void CPhysicsManager::SleepObject( const physobjid_t& obj )
{
    SleepBodyInBlock( obj );
}
void CPhysicsManager::WakeObject( const physobjid_t& obj )
{
    SleepBodyInBlock( obj );
}

void CPhysicsManager::PrepareTick( void )
{
    ResetCollisions();
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

    PrepareTick();
    // Placeholder for physics simulation logic
    // This would typically involve updating positions and velocities of all bodies based on forces, collisions, etc.
    for (blockid_t block_index = 0; block_index < m_block_count; ++block_index)
    {
        CBodyblock& block = m_blocks[block_index];
        for (bodyid_t body_index = 0; body_index < block.GetBodyCount(); ++body_index)
        {
            physobjid_t obj_id = (physobjid_t)((block_index << 16) | body_index);

            #ifdef PHYS_PRINTS
            printf(
                "[BEFORE SimulateBody] obj=%u block=%u body=%u\n"
                "  pos = (%+.9f, %+.9f, %+.9f)\n"
                "  vel = (%+.9f, %+.9f, %+.9f)\n"
                "  ang = (%+.9f, %+.9f, %+.9f)\n",
                obj_id,
                block_index,
                body_index,
                block.x[body_index],
                block.y[body_index],
                block.z[body_index],
                block.vx[body_index],
                block.vy[body_index],
                block.vz[body_index],
                block.wx[body_index],
                block.wy[body_index],
                block.wz[body_index]
            );
            #endif

            SimulateBody(obj_id, delta_time);

            #ifdef PHYS_PRINTS

            printf(
                "[AFTER  SimulateBody] obj=%u block=%u body=%u\n"
                "  pos = (%+.9f, %+.9f, %+.9f)\n"
                "  vel = (%+.9f, %+.9f, %+.9f)\n"
                "  ang = (%+.9f, %+.9f, %+.9f)\n",
                obj_id,
                block_index,
                body_index,
                block.x[body_index],
                block.y[body_index],
                block.z[body_index],
                block.vx[body_index],
                block.vy[body_index],
                block.vz[body_index],
                block.wx[body_index],
                block.wy[body_index],
                block.wz[body_index]
            );
            #endif
        }
    }
    for (blockid_t block_index = 0; block_index < m_block_count; ++block_index)
    {
        CBodyblock& block = m_blocks[block_index];
        bodyid_t block_size = block.GetBodyCount();
        for (bodyid_t body_index = 0; body_index < block_size; ++body_index)
        {
            physobjid_t obj_id = (physobjid_t)((block_index << 16) | body_index);
            collision_event_t world_collision;
            TestCollisionAABBvsPlane( *this, obj_id, AXIS_Y, -4.0f, world_collision );
            for (bodyid_t other = body_index + 1; other < block_size; ++other)
            {
                physobjid_t other_id = (physobjid_t)((block_index << 16) | other);
                collision_event_t ev;
                TestCollisionAABBvsAABB( *this, obj_id, other_id, ev );
            }
        }
    }
#ifdef PHYS_PRINTS
    printf( "contacts: %zu\n", m_collisionEvents.size() );

    for (const collision_event_t& ev : m_collisionEvents)
    {
        printf(
            "contact: %u -> %u, count=%d, normal=(%f,%f,%f)\n",
            ev.a,
            ev.b,
            ev.count,
            ev.normal[0],
            ev.normal[1],
            ev.normal[2]
        );
    }

#endif
    SolveContacts( *this, delta_time );
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

void CPhysicsManager::GetObjectRotation( const physobjid_t& obj, quat_t out )
{
    blockid_t block_id = (blockid_t)((obj & PHYSOBJ_BLOCK_MASK) >> 16);
    bodyid_t body_id = (bodyid_t)(obj & PHYSOBJ_BODY_MASK);
    if (block_id < 0 || block_id >= (blockid_t)m_block_count) return; // Invalid block ID
    CBodyblock& block = m_blocks[block_id];

    if (body_id < 0 || body_id >= (bodyid_t)block.GetBodyCount()) return; // Invalid body ID

    out[0] = block.qx[body_id];
    out[1] = block.qy[body_id];
    out[2] = block.qz[body_id];
    out[3] = block.qw[body_id];
}


void CPhysicsManager::AddForceCentre( const physobjid_t& obj, const vec3_t force )
{
    blockid_t block_id = (blockid_t)((obj & PHYSOBJ_BLOCK_MASK) >> 16);
    bodyid_t body_id = (bodyid_t)(obj & PHYSOBJ_BODY_MASK);
    if (block_id < 0 || block_id >= (blockid_t)m_block_count) return; // Invalid block ID
    CBodyblock& block = m_blocks[block_id];

    if (body_id < 0 || body_id >= (bodyid_t)block.GetBodyCount()) return; // Invalid body ID

    // Store inv-mass?

    block.vx[body_id] += force[0] * block.inv_mass[body_id];
    block.vy[body_id] += force[1] * block.inv_mass[body_id];
    block.vz[body_id] += force[2] * block.inv_mass[body_id];
}