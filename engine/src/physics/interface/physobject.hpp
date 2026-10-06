#pragma once


#include "math/vector.h"

// Do I even use this?
typedef int32_t physobjid_t;

class  IPhysicsObject
{
public:
    virtual void SetPosition( const vec3_t& position );
    virtual void GetPosition( vec3_t& out );

    virtual void SetVelocity( const vec3_t& velocity );
    virtual void GetVelocity( vec3_t& out );
protected:
    physobjid_t m_ref;
};



