#ifndef ENT_LIGHT_H
#define ENT_LIGHT_H

#include "engine/shared/entity/entity.h"

enum { light_spot, light_env, light_point };

typedef struct entlight_t
{
    entitystate_t state;

    int lighttype;

    byte col[4];
    float intensity;

    union 
    {
        struct
        {
            float outradius;
            float inradius;
        } spot;
        
    };

} entlight_t;

#endif
