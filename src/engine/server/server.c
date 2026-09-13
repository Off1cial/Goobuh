#include "network/net.h"
#include "engine/server/server.h"
#include <string.h>

server_t sv_main;
// Cvars - Half-Life convention
float sv_tickrate = 2.0f;
float sv_accelerate = 10.0f;
float sv_airaccelerate = 10.0f;
float sv_stopspeed = 100.0f;
float sv_maxspeed = 320.0f;
float sv_friction = 4.0f;
float sv_gravity = 600.0f;
float sv_maxplayers = 32;


uint8_t sv_init(void) {
  memset(&sv_main, 0, sizeof(sv_main));
  net_addr_from_string("0.0.0.0", 27015, &sv_main.localaddr);
  netaddr_t bindaddr = sv_main.localaddr; // memset above zeroed it — caller sets this first


  sv_main.sock_udp = net_socket_create(NET_PROT_UDP);
  if (sv_main.sock_udp == NET_INVALID_SOCKET) {
    fprintf(stderr, "sv_init: failed to create UDP socket\n");
    return 0;
  }

  sv_main.sock_tcp = net_socket_create(NET_PROT_TCP);
  if (sv_main.sock_tcp == NET_INVALID_SOCKET) {
    fprintf(stderr, "sv_init: failed to create TCP socket\n");
    net_socket_close(sv_main.sock_udp);
    return 0;
  }

  sv_main.localaddr = bindaddr;

  if (net_socket_bind(sv_main.sock_udp, sv_main.localaddr) < 0 ||
      net_socket_bind(sv_main.sock_tcp, sv_main.localaddr) < 0) {
    net_socket_close(sv_main.sock_udp);
    net_socket_close(sv_main.sock_tcp);
    return 0;
  }

  if (net_socket_listen(sv_main.sock_tcp, 16) < 0) {
    net_socket_close(sv_main.sock_udp);
    net_socket_close(sv_main.sock_tcp);
    return 0;
  }

  sv_main.initialised = 1;
  return 1;
}

static svplayer_t *sv_find_client_by_addr(const netaddr_t *addr) {
  for (uint16_t i = 0; i < SV_MAX_PLAYERS; i++) {
    if (sv_main.clients[i].client_state == CS_FREE) continue;
    if (net_addr_eq(&sv_main.clients[i].addr, addr)) return &sv_main.clients[i];
  }
  return NULL;
}

static void sv_accept_new_clients(void) {
  netaddr_t addr;
  netsocket_t client_sock = net_socket_accept(sv_main.sock_tcp, &addr);
  if (client_sock == NET_INVALID_SOCKET) return;

  svplayer_t *slot = NULL;
  for (uint16_t i = 0; i < SV_MAX_PLAYERS; i++) {
    if (sv_main.clients[i].client_state == CS_FREE) {
      slot = &sv_main.clients[i];
      break;
    }
  }

  if (!slot) {
    fprintf(stderr, "sv_think: server full, dropping connection\n");
    net_socket_close(client_sock);
    return;
  }
  printf("Accepting handshake\n");

  memset(slot, 0, sizeof(*slot));
  slot->addr = addr;
  slot->client_state = CS_CONNECTED;
  sv_main.numclients++;

  // TODO: send real serverinfo_t / assigned player id once protocol.h
  // defines the join response. For now: any reply at all == approved.
  byte ack = 1;
  net_send(client_sock, NULL, &ack, sizeof(ack));
  net_socket_close(client_sock); // TCP was only for the handshake — gameplay moves to UDP
}

// Raw struct blit for now, matched to a slot by source addr. No
// sequencing/acks/replay protection — that's netchan_t's job once it
// exists; this just proves cmds arrive.
static void sv_read_client_cmds(void) {
  byte buf[NET_MAX_PACKET];
  netaddr_t from;
  int32_t n;

  while ((n = net_recv(sv_main.sock_udp, &from, buf, sizeof(buf))) > 0) {
    svplayer_t *cl = sv_find_client_by_addr(&from);
    if (!cl) continue; // packet from someone who never completed the TCP handshake

    if (cl->client_state == CS_CONNECTED) cl->client_state = CS_ACTIVE;

    if ((size_t)n != sizeof(usercmd_t)) continue;
    usercmd_t* cmd = (usercmd_t*)buf;
    memcpy(&cl->cmds[cl->last_cmd_tick % SV_PLAYER_HISTORY], buf, sizeof(usercmd_t));
    cl->last_cmd_tick = cmd->tick;
  }
}

void sv_think(void) {
  if (!sv_main.initialised) return;
  sv_accept_new_clients();
  sv_read_client_cmds();
  // TODO: run movement on latest cmds, build snapshots, broadcast to clients.
}
