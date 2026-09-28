#include "public/engine/entity.h"
#include <string.h>

entity_t g_entities[MAX_ENTITIES] = {0};

entity_t* ENT_Alloc( void ){
    for (int i = 0; i < MAX_ENTITIES; i++){
        if (g_entities[i].free) return &g_entities[i];
    }
    return NULL;
}

void ENT_Destroy( entity_t* pEnt){
    memset(pEnt, 0, sizeof(entity_t));
}

