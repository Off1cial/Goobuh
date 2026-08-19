#include "netcode/NetClient.hpp"

bool NetClient::Connect(const NetAddress& addr)
{
  if (!mSocket.Open(NetProtocol::UDP)){
    return false;
  }
  mConnection.SetAddress(addr);
  
  // Construct connection packet
  NetPacketHeader header{};
  header.sequence = 0;
  header.size = sizeof(NetPacketHeader);
  header.type = NetPacketType::Connect;
  ssize_t sent = mSocket.Send(addr, &header, header.size);
  if (sent > 0){
    printf("Connection packet sent\n");
  }

  return true;
}
