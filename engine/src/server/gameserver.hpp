#pragma once

#include "net/networkserver.hpp"

// Make an interface so other games can have their own server logic?

// No networking here, just game logic
class CGameServer : public CNetServer
{
public:
    void Update( void ); // Uses tickrate interval as timestep

private:
};