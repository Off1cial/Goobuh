#pragma once

#include "netcode/NetCommon.hpp"
#include "netcode/NetSocket.hpp"


class NetClient
{
  public:
    NetClient() = default;
    virtual ~NetClient() = default;


    virtual bool Connect(const NetAddress& addr);
    //virtual void Disconnect();
    //virtual void Think();

  private:
    NetSocket mSocket; // UDP, maybe add another for TCP
    NetConnection mConnection;
    double mTimeElapsed;
};
