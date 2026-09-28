#pragma once

#include "math/vector.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ENTITIES 4096

typedef struct model_t model_t;

typedef struct 
{
    vec3_t origin;
    vec3_t velocity;
    qangle angles;
    model_t* model;
} entity_state_t;

typedef struct entity_t
{
    entity_state_t state;
    int free;
    int movtype;
} entity_t;

#define ENT_NUM( index ) (&g_entities[(index)])

extern entity_t g_entities[MAX_ENTITIES];

#ifdef __cplusplus
}
#endif

