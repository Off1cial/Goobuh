#include "entity.h"

#include "physics/physmanager.hpp"
#include "assetmanager.hpp"

CEntity g_Entities[MAX_ENTITIES];


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


bool CEntity::Spawn( vec3_t origin, vec3_t velocity, qangle angles, float mass )
{
    ModelData mdl;
    if (!g_AssetManager->GetModelData( state.model, mdl )) return false;

    m_mass = mass;
    m_physicsEnabled = true;
    m_physobj = g_PhysicsManager->CreatePhysicsObject( origin, velocity, mdl.halfs, angles, mass );
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

void CEntity::OnCollision( void )
{
    printf("Collision\n");
    m_collision = false;
}