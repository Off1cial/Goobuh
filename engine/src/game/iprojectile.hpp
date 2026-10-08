#pragma once

#include "public/engine/entity.hpp"



class IProjectile : public IEntity
{
public:
    virtual void OnImpactWorld( void ) = 0;
    virtual void OnImpactEntity( void ) = 0;

    virtual void Init(); // Initialise the template class


    virtual void Spawn( void ) override = 0;
};