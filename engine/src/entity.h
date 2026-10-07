#pragma once

#include "math/vector.h"
#include "math/quat.h"
#include "public/engine/assethandle.hpp"
#include "public/engine/entity.hpp"
#include <vector>
#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ENTITIES 4096

typedef int32_t physobjid_t;


// Client data
typedef struct entity_state_t
{
    vec3_t origin; // Updated from physics
    quat_t rotation;
    AssetHandle model;
    u32 tex_index; // Temporary, make the model contain the textures
} entity_state_t;



typedef struct entity_s
{
    entity_state_t state;
    bool free;
    bool simulated; // Is this entity ever simulated by the physics engine?
    physobjid_t physobj_id; // The physics object associated with this entity, if any

} entity_t;



inline entity_t* ED_NUM( int index );
inline void ED_INIT( void );

inline void ED_FREE( entity_t* e );


entity_t* ED_NEW( 
    vec3_t origin, 
    vec3_t velocity, 
    qangle angles,
    vec3_t half_sizes, 
    AssetHandle model, 
    u32 tex_index,
    bool simulated,
    float mass
);


class CEntity : public IEntity
{
public:
    CEntity( void ) : free(true) {}

    CEntity( vec3_t origin, vec3_t velocity, vec3_t angles, float mass, AssetHandle model )
    {
        state.model = model;
        Spawn( origin, velocity, angles, mass );
    }
    ~CEntity( void ) = default;
    bool Spawn( vec3_t origin, vec3_t velocity, vec3_t angles, float mass ) override;
    void EnablePhysics( float mass ) override;
    void DisablePhysics( void ) override;
    bool HasPhysics( void ) const { return m_physicsEnabled; }
    void OnCollision( void ) override;
    physobjid_t GetPhysobj( void ) const { return m_physobj; }

    void Destroy( void ) override;


    void SetIndex( size_t i ) { m_index = i; }

    entity_state_t state;
    bool free = true;

private:
    bool m_physicsEnabled = true;
    float m_mass;
    physobjid_t m_physobj = -1;

    size_t m_index; // into global entity array

    bool m_collision = false;

};
extern CEntity g_Entities[MAX_ENTITIES];
CEntity* SpawnEntity( vec3_t origin,  vec3_t velocity,  qangle angles,
                      float mass, AssetHandle model );

#ifdef __cplusplus
}
#endif
