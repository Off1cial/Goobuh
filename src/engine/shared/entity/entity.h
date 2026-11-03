#ifndef ENTITY_H
#define ENTITY_H


#define MAX_ENTITY_NAME 64
#include "math/vector.h"

typedef struct model_t model_t;

// An entity has gameplay properties, engine properties, thinking and usage callbacks

typedef struct 
{
    vec3_t origin;
    vec3_t velocity;
    vec3_t angles;
    model_t *mdl;
} entitystate_t;


typedef struct entity_t
{
    entitystate_t state;

} entity_t;

// Separate gameplay properties from defintions?

typedef struct entdef_t
{
    char name [MAX_ENTITY_NAME]; // e.g player_spawn, player_rocket
    vec3_t halfs;
    int simulated; // Does this entity have physics, or any updates?
} entdef_t;



#endif
