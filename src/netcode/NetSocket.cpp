#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <netinet/in.h>
#include "netcode/NetSocket.hpp"


bool NetSocket::Open(const NetProtocol protocol)
{
  int sock = -1;
  int type = -1;
  switch (protocol)
  {
    case NetProtocol::UDP: {
      type = SOCK_DGRAM;
      break;
    }
    case NetProtocol::TCP: {
      type = SOCK_STREAM;
      break;
    }
    default:
      break;
  }
  sock = socket(AF_INET, type, 0);
  if (sock < 0)
    return false;
  
  mHandle = sock;
  mProtocol = protocol;
  return true;
}

void NetSocket::Close()
{
  close(mHandle);
}


bool NetSocket::Bind(const NetAddress& addr)
{
  return bind(
    mHandle, 
    reinterpret_cast<const sockaddr*>(&addr.Native()),
    sizeof(addr.Native())) == 0;
}

ssize_t NetSocket::Send(const NetAddress& addr, const void* data, const size_t size)
{
  return send(mHandle, data, size, 0);
}

ssize_t NetSocket::Receive(NetAddress& sender, void* data, const size_t size)
{
  sockaddr_in addr{};
  socklen_t addrlen = sizeof(addr);
  
  ssize_t res = recvfrom(
      mHandle,
      data,
      size,
      0,
      reinterpret_cast<sockaddr*>(&addr),
      &addrlen
  );

  if (res >= 0)
    sender = NetAddress::FromNative(addr);

  return res;
}
