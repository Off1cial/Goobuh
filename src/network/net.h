#pragma once
#include <stdint.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// General OS wrapper + networking tools


typedef unsigned char byte;
typedef int32_t netsocket_t;

typedef enum {
  CON_UNITIALISED,
  CON_DISCONNECTED,
  CON_CONNECTED,
  CON_ACTIVE
} constate_t; 


typedef struct {
  int32_t ip;
  uint16_t port;
} netaddr_t;

typedef enum
{
  NET_PROT_UDP,
  NET_PROT_TCP,
} netprot_t;


netaddr_t netaddr_create(char* ip, uint16_t port);

netsocket_t netsocket_create(netprot_t protocol);
void netsocket_close(netsocket_t* sock);

// Binds a socket to a net address, returns 1 on success
int8_t netsocket_bind(netsocket_t sock, netaddr_t addr);
// Listen for N bytes on this socket, returns the number of bytes received
size_t netsocket_listen(netsocket_t sock, size_t n);

