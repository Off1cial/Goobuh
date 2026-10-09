#pragma once

#include "net/networkserver.hpp"
#include "physics/physmanager.hpp"
// Make an interface so other games can have their own server logic?

class CServerPlayer;

// No networking here, just game logic
class CGameServer : public INetServer, public CPhysicsManager
{
public:

    void OnStartup( void ); 

    void Update( void ); // Uses tickrate interval as timestep
    void ChangeMap( const char* pMapName );


    void InitPlayer( CServerPlayer* pPlayer ); // Predictable init behaviour, e.g make physbody
    virtual void SpawnPlayer( CServerPlayer* pPlayer ); // Custom spawn behaviour
private:
    const char* pMapname;
};
