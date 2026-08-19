#include "netcode/NetCommon.hpp"
#include "core/logsys.h"
#include <arpa/inet.h>

NetAddress::NetAddress(u16 port)
{
  mAddr = {};
  mAddr.sin_addr.s_addr = INADDR_ANY;
  mAddr.sin_port = htons(port);
  mAddr.sin_family = AF_INET;
}

NetAddress::NetAddress(const char* ip, u16 port)
{
  mAddr = {};

  
  // IP text -> binary
  if (inet_pton(AF_INET, ip, &mAddr.sin_addr) <= 0){
    LOG_ERROR("Failed to create NetAddress, invalid ip");
    return;
  }

  mAddr.sin_family = AF_INET;
  mAddr.sin_port = htons(port);
}

NetAddress NetAddress::FromNative(const sockaddr_in& addr)
{
  NetAddress res;
  res.mAddr = addr;
  return res;
}


bool NetAddress::operator==(const NetAddress& other) const
{
  sockaddr_in other_addr = other.Native();
  return
    (mAddr.sin_port == other_addr.sin_port && 
     mAddr.sin_addr.s_addr == other_addr.sin_addr.s_addr);
}
