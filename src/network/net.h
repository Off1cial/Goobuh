#pragma once
#include <stdint.h>
#include <stddef.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// General OS wrapper + networking tools

typedef unsigned char byte;
typedef int32_t netsocket_t;

#define NET_INVALID_SOCKET (-1)
#define NET_MAX_PACKET 1400 // stay under typical MTU minus IP/UDP headers

typedef enum {
  CON_UNITIALISED,
  CON_ZOMBIE,
  CON_WAITING, // Waiting for join approval or in queue
  CON_CONNECTED,
  CON_ACTIVE
} constate_t;

typedef struct {
  uint32_t ip;   // host byte order
  uint16_t port; // host byte order
} netaddr_t;

typedef enum {
  NET_PROT_UDP,
  NET_PROT_TCP,
} netprot_t;

// --- socket lifecycle ---
netsocket_t net_socket_create(netprot_t prot);
void        net_socket_close(netsocket_t sock);
int32_t     net_socket_bind(netsocket_t sock, netaddr_t addr);
int32_t     net_socket_set_nonblocking(netsocket_t sock);

// TCP only — used for handshake / serverinfo query, not the gameplay channel
int32_t     net_socket_connect(netsocket_t sock, netaddr_t addr);
int32_t     net_socket_listen(netsocket_t sock, int32_t backlog);
netsocket_t net_socket_accept(netsocket_t sock, netaddr_t *out_addr);

// --- send / receive ---
// UDP: connectionless, `to`/`from` are required.
// TCP: pass NULL for `to`/`from` (socket is already connect()ed/accept()ed).
int32_t net_send(netsocket_t sock, const netaddr_t *to, const byte *data, size_t len);
int32_t net_recv(netsocket_t sock, netaddr_t *from, byte *buf, size_t buflen);

// --- addr helpers ---
void        net_addr_to_sockaddr(const netaddr_t *addr, struct sockaddr_in *out);
void        net_sockaddr_to_addr(const struct sockaddr_in *in, netaddr_t *out);
const char *net_addr_to_string(const netaddr_t *addr, char *buf, size_t buflen);
uint8_t     net_addr_from_string(const char *str, uint16_t port, netaddr_t *out);
uint8_t     net_addr_eq(const netaddr_t *a, const netaddr_t *b);
