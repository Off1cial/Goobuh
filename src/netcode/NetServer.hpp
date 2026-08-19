#pragma once

#include <vector>

#include "netcode/NetCommon.hpp"
#include "netcode/NetSocket.hpp"

#define NET_SERVER_MAX_CLIENTS 64

// Base class to be inherited by other game servers
class NetServer
{
  public:
    NetServer() = default;
    virtual bool Start(u16 port, u16 tickrate);
    virtual void Shutdown();
    virtual void Think();
    virtual void Run();


  private:
    
    virtual NetConnection* FindConnection(const NetAddress& target);
    //virtual void ConnectionResponse();

    NetSocket mSocket; // Receive
    std::array<NetConnection, NET_SERVER_MAX_CLIENTS> mClients; // Send out
    bool mRunning = 0;

    double mTimeElapsed = 0.0;
    u64 mTicks = 0;
    u16 mTickRate = 64;

    int mClientCount = 0;

};
