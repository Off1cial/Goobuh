
#include "common/common.h"
#include "physics/physmanager.hpp"
#include "physics/physobject.hpp"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "math/vector.h"

CPhysicsManager* g_PhysicsManager = new CPhysicsManager();

static FORCEINLINE void body_setflag( CBodyblock* block, bodyid_t id, char flag ){
    block->state_flags[id] |= flag;
}

static FORCEINLINE void body_clearflag( CBodyblock* block, bodyid_t id, char flag ){
    block->state_flags[id] &= ~flag;
}

static FORCEINLINE bool body_hasflag( CBodyblock* block, bodyid_t id, char flag ){
    return (block->state_flags[id] & flag) != 0;
}

bool CBodyblock::Init( void )
{
    allocation = calloc(1, ALLOCATION_SIZE);
    if (!allocation) return false;

    x = (float*)allocation;
    y = (float*)((uintptr_t)x + sizeof(float) * BODY_COUNT);
    z = (float*)((uintptr_t)y + sizeof(float) * BODY_COUNT);

    vx = (float*)((uintptr_t)z + sizeof(float) * BODY_COUNT);
    vy = (float*)((uintptr_t)vx + sizeof(float) * BODY_COUNT);
    vz = (float*)((uintptr_t)vy + sizeof(float) * BODY_COUNT);

    mass = (float*)((uintptr_t)vz + sizeof(float) * BODY_COUNT);

    hx = (float*)((uintptr_t)mass + sizeof(float) * BODY_COUNT);
    hy = (float*)((uintptr_t)hx + sizeof(float) * BODY_COUNT);
    hz = (float*)((uintptr_t)hy + sizeof(float) * BODY_COUNT);

    return true;
}

bodyid_t CBodyblock::AddBody( const vec3_t position, const vec3_t velocity, const float mass_value, const vec3_t halfs )
{
    if (m_body_count >= BODY_COUNT) return INVALID_BODY_ID; // Block is full

    x[m_body_count] = position[0];
    y[m_body_count] = position[1];
    z[m_body_count] = position[2];

    vx[m_body_count] = velocity[0];
    vy[m_body_count] = velocity[1];
    vz[m_body_count] = velocity[2];

    mass[m_body_count] = mass_value;

    hx[m_body_count] = halfs[0];
    hy[m_body_count] = halfs[1];
    hz[m_body_count] = halfs[2];

    m_body_count++;
    return (bodyid_t)(m_body_count - 1);
}

void CBodyblock::Clear( void )
{
    if (allocation)
    {
        free(allocation);
    }
    allocation = nullptr;
    x = y = z = nullptr;
    vx = vy = vz = nullptr;
    mass = nullptr;
    hx = hy = hz = nullptr;
}

void CBodyblock::Reset( void )
{
    if (allocation) memset(allocation, 0, ALLOCATION_SIZE);
}

void CBodyblockManager::Shutdown( void )
{
    for (auto& block : m_blocks)
    {
        block.Clear();
    }
    m_blocks.clear();
    m_block_count = 0;
}

blockid_t CBodyblockManager::CreateBlock( void )
{
    CBodyblock new_block;
    if (!new_block.Init()) return INVALID_BLOCK_ID;
    new_block.m_block_id = (blockid_t)m_block_count;

    m_blocks.push_back(std::move(new_block));
    m_block_count++;
    return (blockid_t)(m_block_count - 1);
}

blockid_t CBodyblockManager::FindBlock( void )
{
    for (size_t i = 0; i < m_block_count; ++i)
    {
        if (m_blocks[i].GetBodyCount() < CBodyblock::BODY_COUNT)
        {
            return (blockid_t)i;
        }
    }
    return CreateBlock(); // No available block, create a new one
}


physobjid_t CBodyblockManager::AddBodyToBlock( blockid_t block, const vec3_t position, const vec3_t velocity, const float mass, const vec3_t halfs )
{
    if (block < 0 || block >= (blockid_t)m_block_count) return INVALID_PHYSOBJ_ID; // Invalid block ID
    bodyid_t body_id = m_blocks[block].AddBody(position, velocity, mass, halfs);
    if (body_id == INVALID_BODY_ID) return INVALID_PHYSOBJ_ID; // Block is full, cannot add body
    return (physobjid_t)((block << 16) | body_id); // Combine block and body IDs into a single physobjid_t
}

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
    printf("Simulating body %d in block %d: New position = (%f, %f, %f)\n", body_id, block_id, block.x[body_id], block.y[body_id], block.z[body_id]);

    // Apply gravity if enabled
    // This is a placeholder; actual gravity application would depend on the physics system's design
    block.vy[body_id] -= PHYS_DEFAULT_GRAVITY * delta_time; // Simple gravity effect
}

void CPhysicsManager::Simulate( float delta_time )
{
    // Placeholder for physics simulation logic
    // This would typically involve updating positions and velocities of all bodies based on forces, collisions, etc.
    for (size_t block_index = 0; block_index < m_block_count; ++block_index)
    {
        CBodyblock& block = m_blocks[block_index];
        for (size_t body_index = 0; body_index < block.GetBodyCount(); ++body_index)
        {
            physobjid_t obj_id = (physobjid_t)((block_index << 16) | body_index);
            SimulateBody(obj_id, delta_time);
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
