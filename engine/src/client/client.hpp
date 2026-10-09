#pragma once

#include "net/networkclient.hpp"

class IInput;

class CGameClient
{
public:
        
    virtual void ThinkFrames( void ); // runs every iteration
    virtual void ThinkTicks( void ); // runs at a tick rate

    void JoinServer( const char* ip, u16 port );
    void Disconnect( const char* reason, ... );
protected:
    INetClient* m_pNetClient; // Not always online
    IInput* m_pInput; 


};

