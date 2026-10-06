#include "physics/interface/physobject.hpp"
#include "physics/physmanager.hpp"

void IPhysicsObject::SetPosition( const vec3_t& position )
{
    g_PhysicsManager->SetObjectPosition( m_ref, position );
}