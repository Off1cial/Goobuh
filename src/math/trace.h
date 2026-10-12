#pragma once


#include "math/vector.h"
#include "common/common.h"


typedef struct entity_t entity_t;

struct trace_t
{
    vec3_t start;
    vec3_t end;
    bool hit = false;
    float fraction = 0.0f; // 0 - 1 along the ( start - end ) vector
    entity_t* target = nullptr;
};