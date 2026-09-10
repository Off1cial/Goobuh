#include "network/net.h"

#include <stdint.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>

netsocket_t netsocket_create(netprot_t protocol){
  int type = (protocol == NET_PROT_UDP) ? SOCK_DGRAM : SOCK_STREAM; 
  return socket(AF_INET, type, 0);
}

void netsocket_close(netsocket_t* sock){
  close(*sock);
  *sock = -1;
}

int8_t netsocket_bind(netsocket_t sock, netaddr_t addr){
  struct sockaddr_in in = {
    .sin_family = AF_INET,
    .sin_port = htons(addr.port),
    .sin_addr.s_addr = addr.ip
  };
  return bind(sock, (struct sockaddr*)&in, sizeof(in)) >= 0;
}


size_t netsocket_listen(netsocket_t sock, size_t n){
  int rec = listen(sock, n);
  return (rec <= 0) ? 0 : (size_t)rec;
}



netaddr_t netaddr_create(char* ip, uint16_t port){

  netaddr_t addr =  {
  .ip = (int32_t)inet_pton(AF_INET, ip, &addr.ip),
  .port = htons(port)};

  return addr;
}
