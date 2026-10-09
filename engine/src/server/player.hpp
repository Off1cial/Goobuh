#pragma once

#include "math/vector.h"
#include "net/networkserver.hpp"
#include "public/engine/entity.hpp"


struct playerstate_t
{
    vec3_t origin;
    vec3_t velocity;
    qangle viewangles;
    // Command history?
};



class CServerPlayer : public INetServerClient
{
public:
    CServerPlayer( void );
    void JoinTeam( int team ); // Example
    void JoinClass( int gameclass );

    void Init( void );

    int32_t physbody = -1;

    playerstate_t state;
private:
    
};
