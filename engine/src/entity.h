#pragma once

#include "math/vector.h"
#include "math/quat.h"
#include "public/engine/assethandle.hpp"
#include "public/engine/entity.hpp"
#include <vector>

#define MAX_ENTITIES 4096

typedef int32_t physobjid_t;

#ifdef __cplusplus
extern "C" {
#endif

// Client data
    typedef struct entity_state_t
    {
        vec3_t origin;      // Updated from physics
        quat_t rotation;
        AssetHandle model;
        vec3_t size;        // Absolute size in world units
        vec3_t draw_scale;  // Derived: size / native model size (renderer only)
        u32 tex_index;      // Temporary
    } entity_state_t;

    typedef struct entity_s
    {
        entity_state_t state;
        bool free;
        bool simulated;
        physobjid_t physobj_id;
    } entity_t;

    inline entity_t* ED_NUM( int index );
    inline void ED_INIT( void );
    inline void ED_FREE( entity_t* e );

    entity_t* ED_NEW( vec3_t origin, vec3_t velocity, qangle angles,
                      vec3_t half_sizes, AssetHandle model, u32 tex_index,
                      bool simulated, float mass );

#ifdef __cplusplus
}   // end extern "C"
#endif

// Everything below is C++ only
struct ModelData;

class CEntity : public IEntity
{
public:
    CEntity( void ) : free( true ) {}

    // Native model size
    CEntity( vec3_t origin, vec3_t velocity, vec3_t angles, float mass, bool gravity, AssetHandle model, AssetHandle texture )
    {
        state.model = model;
        Spawn( origin, velocity, angles, mass );
        if (!gravity) DisableGravity();
    }

    // Absolute size
    CEntity( vec3_t origin, vec3_t velocity, vec3_t angles, float mass, bool gravity, AssetHandle model, AssetHandle texture, vec3_t size )
    {
        state.model = model;
        SetRequestedSize( size );
        Spawn( origin, velocity, angles, mass );
        if (!gravity) DisableGravity();
    }

    ~CEntity( void ) = default;

    bool Spawn( vec3_t origin, vec3_t velocity, vec3_t angles, float mass ) override;
    void EnablePhysics( float mass ) override;
    void DisablePhysics( void ) override;

    void DisableGravity( void );
    void EnableGravity( void );

    bool HasPhysics( void ) const { return m_physicsEnabled; }
    void OnCollision( void ) override;
    physobjid_t GetPhysobj( void ) const { return m_physobj; }
    void Destroy( void ) override;

    void SetIndex( size_t i ) { m_index = i; }

    void SetRequestedSize( const float* size );
    void ClearRequestedSize( void ) { m_hasCustomSize = false; }
    void ResolveSize( const ModelData& mdl );

    entity_state_t state;
    bool free = true;

private:
    bool m_hasCustomSize = false;   // false = use the model's native size
    bool m_physicsEnabled = true;
    float m_mass;
    physobjid_t m_physobj = -1;
    size_t m_index;
    bool m_collision = false;
};

extern CEntity g_Entities[MAX_ENTITIES];

CEntity* SpawnEntity( vec3_t origin, vec3_t velocity, qangle angles,
                      float mass, bool gravity, AssetHandle model, AssetHandle texture );

CEntity* SpawnEntity( vec3_t origin, vec3_t velocity, qangle angles,
                      float mass, bool gravity, AssetHandle model, AssetHandle texture, vec3_t size );