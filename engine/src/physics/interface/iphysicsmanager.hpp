#pragma  once

#include "physics/shared.h"
#include "math/vector.h"

class IPhysicsManager
{
public:
    
   virtual physobjid_t CreatePhysicsObject(
           const vec3_t origin,
           const vec3_t velocity,
           const vec3_t halfs,
           const vec3_t angles,
           const float mass
           ) = 0;

};
