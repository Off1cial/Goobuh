#pragma once

#include "public/engine/input.hpp"
#include "public/engine/entity.hpp"

#define PMVAR_MAXSPEED 320.0F
#define PMVAR_ACCELERATION 40.0F

#define PMVAR_AIRACCELERATION 80.0F

// Player move variables
struct playermvars_t
{
    // Set all to defaults
    void Reset( void )
    {
        maxspeed = PMVAR_MAXSPEED;
        acceleration = PMVAR_ACCELERATION;

        air_acceleration = PMVAR_AIRACCELERATION;
    }

// Ground
    float maxspeed;
    float acceleration;
// Air
    float air_acceleration;
};

class IPlayer : public IEntity, public IInput
{
public:
    virtual ~IPlayer() = default;

    virtual void Think( void ) = 0;

private:
    playermvars_t* m_pMovevars = nullptr;
};