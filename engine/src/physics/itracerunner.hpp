#pragma once

#include "math/vector.h"

// Idea, a class that takes the list of entities and the world data from the engine/map manager and sweeps traces for the physics/weapons
typedef struct trace_t trace_t;
typedef struct entity_t entity_t;

class ITraceRunner
{
public:
    virtual trace_t SweepTrace( vec3_t start, vec3_t end, float max_dist ) = 0;


    entity_t* m_entities;

};