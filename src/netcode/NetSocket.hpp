#pragma once
#include "core/common.h"
#include "netcode/NetCommon.hpp"

class NetSocket
{
  public:
    NetSocket() = default;
    ~NetSocket() = default;

    bool Open(const NetProtocol protcol);
    void Close();

    bool Bind(const NetAddress& addr);
    ssize_t Send(const NetAddress& addr, const void* data, const size_t size);
    ssize_t  Receive(NetAddress& sender, void* data, const size_t size);

  private:
    i32 mHandle = -1;
    NetProtocol mProtocol;
};

enum class ConnectionState : u8
{
  Offline,
  Online,
  Zombie, // Not responding but dont close yet
};  

class NetConnection
{
public:
    NetConnection() = default;
    ~NetConnection() = default;


    void Reset();
    void Update(double time);
  
    /*
    bool Send(
        NetPacketType type,
        const NetBuffer& payload);

    
    void Receive(
        const NetPacketHeader& header,
        NetBuffer& payload);
      */

    bool IsConnected() const { return mConnected; }; 

    const NetAddress& Address() const { return mAddress; };
    void SetAddress(const NetAddress& addr) { mAddress = addr; }

private:
    NetAddress mAddress;
    uint32_t mNextSequence = 0;
    uint32_t mLastReceivedSequence = 0;

    double mLastReceivedTime = 0.0;

    bool mConnected = false;
};
