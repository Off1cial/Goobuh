#include "physics/backend/blockmanager.hpp"
#include "common/common.h"
#include "math/quat.h"
#include "physics/shared.h"
#include <stdio.h>

static FORCEINLINE void body_setflag( CBodyblock* block, bodyid_t id, char flag ){
    block->state_flags[id] |= flag;
}

static FORCEINLINE void body_clearflag( CBodyblock* block, bodyid_t id, char flag ){
    block->state_flags[id] &= ~flag;
}

static FORCEINLINE bool body_hasflag( CBodyblock* block, bodyid_t id, char flag ){
    return (block->state_flags[id] & flag) != 0;
}



/*
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

    wx = (float*)((uintptr_t)vz + sizeof( float ) * BODY_COUNT);
    wy = (float*)((uintptr_t)wx + sizeof( float ) * BODY_COUNT);
    wz = (float*)((uintptr_t)wy + sizeof( float ) * BODY_COUNT);

    mass = (float*)((uintptr_t)vz + sizeof(float) * BODY_COUNT);

    hx = (float*)((uintptr_t)mass + sizeof(float) * BODY_COUNT);
    hy = (float*)((uintptr_t)hx + sizeof(float) * BODY_COUNT);
    hz = (float*)((uintptr_t)hy + sizeof(float) * BODY_COUNT);

    qx = (float*)((uintptr_t)hz + sizeof( float ) * BODY_COUNT);
    qy = (float*)((uintptr_t)qx + sizeof( float ) * BODY_COUNT);
    qz = (float*)((uintptr_t)qy + sizeof( float ) * BODY_COUNT);
    qw = (float*)((uintptr_t)qz + sizeof( float ) * BODY_COUNT);
    return true;
}
*/
bool CBodyblock::Init( void )
{
    allocation = calloc( 1, ALLOCATION_SIZE );
    if (!allocation) return false;

    float** arrays[] = { &x,&y,&z, &vx,&vy,&vz, &wx,&wy,&wz, &inv_mass,
                         &hx,&hy,&hz, &qx,&qy,&qz,&qw };
    float* p = (float*)allocation;
    for (float** a : arrays) { *a = p; p += BODY_COUNT; }

    state_flags = (char*)p;   // after the 17 float arrays
    memset( state_flags, BODYSTATE_FLAG_FREE, BODY_COUNT );
    return true;
}

bool CBodyblock::FindSlot( bodyid_t& out )
{
    for (int i = 0; i < BODY_COUNT; i++){
        if (body_hasflag( this, i, BODYSTATE_FLAG_FREE)){
            out = i;
            return true;
        }
    }
    return false;
}

void CBodyblock::SleepBody( bodyid_t body )
{
    body_setflag( this, body, BODYSTATE_FLAG_SLEEP);
}

void CBodyblock::WakeBody( bodyid_t body )
{
    body_clearflag( this, body, BODYSTATE_FLAG_SLEEP );
}


bool CBodyblock::AddBody( bodyid_t& id_out, const vec3_t position, const vec3_t velocity, const float mass_value, const vec3_t halfs, const qangle angles )
{
    //if (slot >= BODY_COUNT) return INVALID_BODY_ID; // Block is full
    bodyid_t slot;
    if (!FindSlot( slot )){
        return false;
    }



    x[slot] = position[0];
    y[slot] = position[1];
    z[slot] = position[2];

    vx[slot] = velocity[0];
    vy[slot] = velocity[1];
    vz[slot] = velocity[2];

    inv_mass[slot] = mass_value > 0 ? 1.0f / mass_value : 0.0f;

    hx[slot] = halfs[0];
    hy[slot] = halfs[1];
    hz[slot] = halfs[2];

    wx[slot] = 0;
    wy[slot] = 0;
    wz[slot] = 0;

    quat_t quat;
    QuatFromAngles(angles, quat );
    qx[slot] = quat[0];
    qy[slot] = quat[1];
    qz[slot] = quat[2];
    qw[slot] = quat[3];


    id_out = slot;
    body_clearflag( this, slot, BODYSTATE_FLAG_FREE );
    m_body_count++;
    return true;
}


void CBodyblock::RemoveBody( bodyid_t body_id ) // Mark the spot as free, wont be simulated
{
    body_setflag( this, body_id, BODYSTATE_FLAG_FREE );
    m_body_count--;
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
    inv_mass = nullptr;
    hx = hy = hz = nullptr;
}

void CBodyblock::Reset( void )
{
    if (allocation) memset(allocation, 0, ALLOCATION_SIZE);
    for (int i = 0; i < BODY_COUNT; i++){
        body_setflag( this, i, BODYSTATE_FLAG_FREE);
    }
}

static FORCEINLINE bool valid_bodyid( const CBodyblock& block, bodyid_t id )
{
    return id >= 0 && id < block.BODY_COUNT;
}
// Use very carefully
#define BODY_IDCHECK() \
    if (!valid_bodyid(*this, body_id)) {return;}
#define BODY_GETVEC3(body, a0, a1, a2, out)  out[0] = a0[body]; out[1] = a1[body]; out[2] = a2[body]
#define BODY_SETVEC3(body, a0, a1, a2, vec)  a0[body] = vec[0]; a1[body] = vec[1]; a2[body] = vec[2]

void CBodyblock::GetMass( bodyid_t body_id, float& out )
{
    BODY_IDCHECK();
    out = 1.0f / inv_mass[body_id];
}

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
void CBodyblock::GetVelocity( bodyid_t body_id, vec3_t out )
{
    BODY_IDCHECK();
    BODY_GETVEC3(body_id, vx, vy, vz, out );
}

void CBodyblock::SetVelocity( bodyid_t body_id, vec3_t velocity )
{
    BODY_IDCHECK();
    BODY_SETVEC3( body_id, vx, vy, vz, velocity );
}

void CBodyblock::GetHalfs( bodyid_t body_id, vec3_t out )
{
    BODY_IDCHECK();
    BODY_GETVEC3( body_id, hx, hy, hz, out );
}

void CBodyblock::GetAABB( bodyid_t body_id, vec3_t pos, vec3_t halfs )
{
    BODY_IDCHECK();
    BODY_GETVEC3( body_id, x, y, z, pos );
    BODY_GETVEC3( body_id, hx, hy, hz, halfs );
}

void CBodyblock::GetRotation( bodyid_t body_id, vec3_t out )
{
    BODY_IDCHECK();;
    BODY_GETVEC3( body_id, qx, qy, qz, out );
}

void CBodyblock::EnableGravity( bodyid_t body_id )
{
    body_clearflag( this, body_id, BODYSTATE_FLAG_GRAVITY_DISABLED);
}

void CBodyblock::DisableGravity( bodyid_t body_id )
{
    body_setflag( this, body_id, BODYSTATE_FLAG_GRAVITY_DISABLED );
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


physobjid_t CBodyblockManager::AddBodyToBlock( blockid_t block, const vec3_t position, const vec3_t velocity, const float mass, const vec3_t halfs, const qangle angles )
{
    if (block < 0 || block >= (blockid_t)m_block_count) return INVALID_PHYSOBJ_ID; // Invalid block ID
    bodyid_t body_id;
    if (!m_blocks[block].AddBody(body_id, position, velocity, mass, halfs, angles)){
        return INVALID_PHYSOBJ_ID;
    }
   
    if (body_id == INVALID_BODY_ID) return INVALID_PHYSOBJ_ID; // Block is full, cannot add body
    return (physobjid_t)((block << 16) | body_id); // Combine block and body IDs into a single physobjid_t
}



#ifndef OBJ_ID_SEPARATE
#define OBJ_ID_SEPARATE( obj, bodyout, blockout ) \
    blockout = (blockid_t)(obj >> 16);\
    bodyout = (bodyid_t)(obj & PHYSOBJ_BODY_MASK)
#endif


void CBodyblockManager::RemoveBodyFromBlock( physobjid_t obj )
{
    blockid_t block;
    bodyid_t body;
    OBJ_ID_SEPARATE( obj, body, block );
    if (block < 0 || block >= (blockid_t)m_block_count) return; // Invalid block ID

    m_blocks[block].RemoveBody(body);
}


void CBodyblockManager::EnableFlag(physobjid_t obj, char flag)
{
    blockid_t block;
    bodyid_t body;
    OBJ_ID_SEPARATE( obj, body, block);
    if (block < 0 || block >= (blockid_t)m_block_count) return; // Invalid block ID
   
    m_blocks[block].EnableFlag( body, flag );

}
void CBodyblockManager::DisableFlag(physobjid_t obj, char flag)
{
    blockid_t block;
    bodyid_t body;
    OBJ_ID_SEPARATE( obj, body, block);
    if (block < 0 || block >= (blockid_t)m_block_count) return; // Invalid block ID
   
    m_blocks[block].DisableFlag( body, flag );

}

void CBodyblockManager::SleepBodyInBlock( physobjid_t obj )
{
    blockid_t block;
    bodyid_t body;
    OBJ_ID_SEPARATE( obj, body, block );
    if (block < 0 || block >= (blockid_t)m_block_count) return; // Invalid block ID

    m_blocks[block].SleepBody( body );

}
void CBodyblockManager::WakeBodyInBlock( physobjid_t obj )
{
    blockid_t block;
    bodyid_t body;
    OBJ_ID_SEPARATE( obj, body, block );
    if (block < 0 || block >= (blockid_t)m_block_count) return; // Invalid block ID


    m_blocks[block].WakeBody( body );
}

void CBodyblockManager::DisableGravity( physobjid_t obj )
{
    blockid_t block;
    bodyid_t body;
    OBJ_ID_SEPARATE( obj, body, block );
    if (block < 0 || block >= (blockid_t)m_block_count) return; // Invalid block ID

    m_blocks[block].DisableGravity( body );
}

void CBodyblockManager::EnableGravity( physobjid_t obj )
{
    blockid_t block;
    bodyid_t body;
    OBJ_ID_SEPARATE( obj, body, block );
    if (block < 0 || block >= (blockid_t)m_block_count) return; // Invalid block ID

    m_blocks[block].EnableGravity( body );
}

bool CBodyblockManager::GetMass( physobjid_t obj, float& out )
{
    blockid_t block; bodyid_t body;
    OBJ_ID_SEPARATE( obj, body, block );
    if (block < 0 || block >= (blockid_t)m_block_count) return false;
    m_blocks[block].GetMass( body, out );
    return true;
}

bool CBodyblockManager::GetHalfs( physobjid_t obj, vec3_t out )
{
    blockid_t block; bodyid_t body;
    OBJ_ID_SEPARATE( obj, body, block );
    if (block < 0 || block >= (blockid_t)m_block_count) return false;

    m_blocks[block].GetHalfs( body, out );
    return true;
}

bool CBodyblockManager::GetPosition( physobjid_t obj, vec3_t out )
{
    blockid_t block; bodyid_t body;
    OBJ_ID_SEPARATE( obj, body, block );
    if (block < 0 || block >= (blockid_t)m_block_count) return false;

    m_blocks[block].GetPos( body, out );
    return true;
}

bool CBodyblockManager::GetVelocity( physobjid_t obj, vec3_t out )
{
    blockid_t block; bodyid_t body;
    OBJ_ID_SEPARATE( obj, body, block );
    if (block < 0 || block >= (blockid_t)m_block_count) return false;

    m_blocks[block].GetVelocity( body, out );
    return true;
}
bool CBodyblockManager::SetVelocity( physobjid_t obj, vec3_t velocity )
{
    blockid_t block; bodyid_t body;
    OBJ_ID_SEPARATE( obj, body, block );
    if (block < 0 || block >= (blockid_t)m_block_count) return false;

    m_blocks[block].SetVelocity( body, velocity );
    return true;
}

bool CBodyblockManager::GetAABB( physobjid_t obj, vec3_t pos, vec3_t halfs )
{
    blockid_t block; bodyid_t body;
    OBJ_ID_SEPARATE( obj, body, block );
    if (block < 0 || block >= (blockid_t)m_block_count) return false;
    #ifdef PHYS_PRINTS
    printf( "GetAABB obj=%u block=%d body=%d count=%d\n", obj, (int)block, (int)body, (int)m_blocks[block].GetBodyCount() );
    #endif
    m_blocks[block].GetAABB( body, pos, halfs );
    return true;
}

// blockmanager.cpp
bool CBodyblockManager::GetOBB( physobjid_t obj, vec3_t pos, vec3_t halfs, vec3_t axes[3] )
{
    blockid_t block; bodyid_t body;
    OBJ_ID_SEPARATE( obj, body, block );
    if (block < 0 || block >= (blockid_t)m_block_count) return false;

    CBodyblock& b = m_blocks[block];
    b.GetAABB( body, pos, halfs );

    quat_t q = { b.qx[body], b.qy[body], b.qz[body], b.qw[body] };
    mat4 m;
    QuatToMatrix( q, m );               // column-major: axis i = column i
    for (int i = 0; i < 3; i++)
        VectorSet( axes[i], m[4 * i], m[4 * i + 1], m[4 * i + 2] );
    return true;
}

bool CBodyblockManager::GetRotation( physobjid_t obj, vec3_t out )
{
    blockid_t block; bodyid_t body;
    OBJ_ID_SEPARATE( obj, body, block );
    if (block < 0 || block >= (blockid_t)m_block_count) return false;

    m_blocks[block].GetRotation( obj, out );
    return true;
}


#ifndef OBJ_ID_SEPARATE
#undef OBJ_ID_SEPARATE
#endif
