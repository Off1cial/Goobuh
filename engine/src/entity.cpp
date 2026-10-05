#include "entity.h"

entity_t g_Entities[MAX_ENTITIES];



inline entity_t* ED_NUM( int index )
{
    return ( (index < 0 || index >= MAX_ENTITIES) ? NULL : &g_Entities[index]);
}

entity_t* ED_ALLOC( void )
{
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        if (!g_Entities[i].free)
            continue;

        memset(&g_Entities[i], 0, sizeof(entity_t));
        g_Entities[i].free = false;
        return &g_Entities[i];
    }
}

inline void ED_FREE( entity_t* e )
{
    if (e) e->free = true;
}

inline void ED_INIT( void )
{
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        g_Entities[i].free = true;
    }
}

entity_t* ED_NEW( vec3_t origin, vec3_t velocity, qangle angles, AssetHandle model )
{
    entity_t* e = ED_ALLOC();
    if (!e) return NULL;

    VectorCopy( origin, e->state.origin );
    VectorCopy( angles, e->state.angles );
    VectorCopy( velocity, e->velocity );
    e->state.model = model;

    return e;
}