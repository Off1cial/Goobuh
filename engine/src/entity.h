#pragma once

#include "math/vector.h"
#include "public/engine/assethandle.hpp"
#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ENTITIES 4096



// Client data
typedef struct entity_state_t
{
    vec3_t origin;
    qangle angles;
    AssetHandle model;
} entity_state_t;


typedef struct entity_s
{
    vec3_t velocity;
    entity_state_t state;
    bool free;

} entity_t;

extern entity_t g_Entities[MAX_ENTITIES];

inline entity_t* ED_NUM( int index );
inline void ED_INIT( void );

inline void ED_FREE( entity_t* e );


entity_t* ED_NEW( vec3_t origin, vec3_t velocity, qangle angles, AssetHandle model );

#ifdef __cplusplus
}
#endif
