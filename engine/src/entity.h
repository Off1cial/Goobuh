#pragma once

#include "math/vector.h"
#include "public/engine/assethandle.hpp"
#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ENTITIES 4096

typedef int32_t physobjid_t;


// Client data
typedef struct entity_state_t
{
    vec3_t origin; // Updated from physics
    qangle angles;
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

extern entity_t g_Entities[MAX_ENTITIES];

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

#ifdef __cplusplus
}
#endif
