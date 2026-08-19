#include "netcode/NetServer.hpp"
#include "core/logsys.h"


NetConnection* NetServer::FindConnection(const NetAddress& target)
{
  for (NetConnection& client : mClients){
    if (client.IsConnected() &&
        client.Address() == target){
      return &client;
    }
  } 
  return nullptr;
}

bool NetServer::Start(u16 port, u16 tickrate)
{
  mTickRate = tickrate;

  if (!mSocket.Open(NetProtocol::UDP)){
    LOG_ERROR("Failed to create server socket");
    return false;
  }
  NetAddress address(port);
  if (!mSocket.Bind(address)){
    LOG_ERROR("Failed to bind server socket");
    mSocket.Close();
    return false;
  }
  mRunning = 1;
  printf("[SERVER]: %dHz server started on port %d\n", tickrate, port);
  return true;
}

void NetServer::Think()
{
  u8 net_buffer[NET_PACKET_MAX_SIZE];
  NetAddress client;

  ssize_t rec = mSocket.Receive(client, net_buffer, sizeof(net_buffer));
  if (rec < (ssize_t)sizeof(NetPacketHeader)){
    return;
  }

  printf("Inspecting packet\n");
  // Inspect packet
}



void NetServer::Run()
{
  while(mRunning){
    Think();
  }
}


void NetServer::Shutdown()
{
  // Send each client a shutdown message
  mSocket.Close();
  mRunning = 0;
}
