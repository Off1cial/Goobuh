#include "entity.h"

#include "physics/physmanager.hpp"
#include "assetmanager.hpp"

CEntity g_Entities[MAX_ENTITIES];

/*
CEntity* SpawnEntity( vec3_t origin, vec3_t velocity, qangle angles,
                      float mass, AssetHandle model )
{
    for (size_t i = 0; i < MAX_ENTITIES; i++)
    {
        CEntity& e = g_Entities[i];
        if (!e.free) continue;
        e.SetIndex( i );
        e.state.model = model;
        QuatFromAngles( angles, e.state.rotation );
        e.free = false;
        e.Spawn( origin, velocity, angles, mass );   // Spawn should write e.state.origin etc.
        return &e;
    }
    return nullptr;
}
*/



// entity.cpp
static CEntity* SpawnEntityInternal( vec3_t origin, vec3_t velocity, qangle angles,
                                     float mass, bool gravity, AssetHandle model, AssetHandle texture, const float* size )
{
    for (size_t i = 0; i < MAX_ENTITIES; i++)
    {
        CEntity& e = g_Entities[i];
        if (!e.free) continue;

        TextureData tex_data;
        g_AssetManager->GetTextureData( texture, tex_data );

        e.SetIndex( i );
        e.state.model = model;
        e.state.tex_index = tex_data.vk_index;
        QuatFromAngles( angles, e.state.rotation );

        if (size) e.SetRequestedSize( size );
        else      e.ClearRequestedSize();   // slots are reused, so always reset

        e.free = false;
        e.Spawn( origin, velocity, angles, mass );
        if (!gravity) e.DisableGravity();
        return &e;
    }
    return nullptr;
}


CEntity* SpawnEntity( vec3_t origin, vec3_t velocity, qangle angles,
                      float mass, bool gravity, AssetHandle model, AssetHandle texture )
{
    return SpawnEntityInternal( origin, velocity, angles, mass, gravity, model, texture, nullptr );
}

CEntity* SpawnEntity( vec3_t origin, vec3_t velocity, qangle angles,
                      float mass, bool gravity, AssetHandle model, AssetHandle texture, vec3_t size )
{
    return SpawnEntityInternal( origin, velocity, angles, mass, gravity, model, texture, size );
}



void CEntity::SetRequestedSize( const float* size )
{
    VectorCopy( size, state.size );
    m_hasCustomSize = true;
}

void CEntity::ResolveSize( const ModelData& mdl )
{
    for (int i = 0; i < 3; ++i)
    {
        float native = mdl.halfs[i] * 2.0f;

        if (!m_hasCustomSize)
            state.size[i] = native;

        state.draw_scale[i] = (native > 1e-6f) ? state.size[i] / native : 1.0f;
    }
}

bool CEntity::Spawn( vec3_t origin, vec3_t velocity, qangle angles, float mass )
{
    ModelData mdl;
    if (!g_AssetManager->GetModelData( state.model, mdl )) return false;

    ResolveSize( mdl );

    vec3_t halfs = { state.size[0] * 0.5f, state.size[1] * 0.5f, state.size[2] * 0.5f };

    m_mass = mass;
    m_physicsEnabled = true;
    m_physobj = g_PhysicsManager->CreatePhysicsObject( origin, velocity, halfs, angles, mass );
    if (m_physobj == INVALID_PHYSOBJ_ID) return false;

    VectorCopy( origin, state.origin );
    QuatFromAngles( angles, state.rotation );
    return true;
}









void CEntity::Destroy( void )
{
    // Effects... all kinds of stuff
    g_Entities[m_index].free = true;



}

void CEntity::EnablePhysics( float mass )
{
    m_physicsEnabled = true;
    m_mass = mass;
    g_PhysicsManager->WakeObject( m_physobj );
}

void CEntity::DisablePhysics( void )
{
    m_physicsEnabled = false;
    g_PhysicsManager->SleepObject( m_physobj );
}

void CEntity::DisableGravity( void )
{
    g_PhysicsManager->DisableGravity( m_physobj );
}

void CEntity::EnableGravity( void )
{
    g_PhysicsManager->EnableGravity( m_physobj );
}


void CEntity::OnCollision( void )
{
    printf("Collision\n");
    m_collision = false;
}