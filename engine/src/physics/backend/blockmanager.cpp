#include "physics/backend/blockmanager.hpp"
#include "common/common.h"


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

static FORCEINLINE bool valid_bodyid( const CBodyblock& block, bodyid_t id ){
    return (id < 0 || id > block.GetBodyCount());
}

// Use very carefully
#define BODY_IDCHECK() \
    if (!valid_bodyid(*this, body_id)) {return;}
#define BODY_GETVEC3(body, a0, a1, a2, out)  out[0] = a0[body]; out[1] = a1[body]; out[2] = a2[body]
#define BODY_SETVEC3(body, a0, a1, a2, vec)  a0[body] = vec[0]; a1[body] = vec[1]; a2[body] = vec[2]

void CBodyblock::GetPos( bodyid_t body_id, vec3_t out )
{
    BODY_IDCHECK();
    BODY_GETVEC3( body_id, x, y, z, out );
}
void CBodyblock::SetPos( bodyid_t body_id, vec3_t pos )
{
    BODY_IDCHECK();
    BODY_SETVEC3( body_id, x, y, z, pos );
}

// All the other getter/setters...


void CBodyblock::GetHalfs( bodyid_t body_id, vec3_t out )
{
    BODY_IDCHECK();
    BODY_GETVEC3( body_id, hx, hy, hz, out );
}

#undef BODY_IDCHECK

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

#ifndef OBJ_ID_SEPARATE
#define OBJ_ID_SEPARATE( obj, bodyout, blockout ) \
    blockout = (blockid_t)(obj >> 16);\
    bodyout = (bodyid_t)(obj && PHYSOBJ_BODY_MASK)
#endif

bool CBodyblockManager::GetHalfs( physobjid_t obj, vec3_t out )
{
    blockid_t block; bodyid_t body;
    OBJ_ID_SEPARATE( obj, body, block );
    if (block < 0 || block >= (blockid_t)m_block_count) return false;

    m_blocks[block].GetHalfs( body, out );
}

bool CBodyblockManager::GetPosition( physobjid_t obj, vec3_t out )
{
    blockid_t block; bodyid_t body;
    OBJ_ID_SEPARATE( obj, body, block );
    if (block < 0 || block >= (blockid_t)m_block_count) return false;

    m_blocks[block].GetPos( body, out );
}

#ifndef OBJ_ID_SEPARATE
#undef OBJ_ID_SEPARATE
#endif