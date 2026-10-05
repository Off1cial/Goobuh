#pragma once

#include "math/vector.h"

class CPhysBox
{
public:
    bool Init( vec3_t origin, vec3_t velocity, vec3_t halfs, float mass ); 


    void AddImpulse( vec3_t impulse, vec3_t contact_vector );

private:
    int m_simdindex;
};