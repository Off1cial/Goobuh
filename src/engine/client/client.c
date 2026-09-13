#include "network/net.h"
#include "engine/client/client.h"
#include "engine/client/cl_app.h"
#include <string.h>

client_t cl_main;

float cl_updaterate = 20.0f; // updates/sec requested from the server
float cl_interp = 0.1f;      // seconds of interpolation buffer

static netaddr_t cl_server_addr;

static void update_cmd(usercmd_t* cmd);

uint8_t cl_init(void) {
  memset(&cl_main, 0, sizeof(cl_main));

  cl_main.conn.socket_udp = net_socket_create(NET_PROT_UDP);
  if (cl_main.conn.socket_udp == NET_INVALID_SOCKET) {
    fprintf(stderr, "cl_init: failed to create UDP socket\n");
    return 0;
  }

  cl_main.conn.socket_tcp = net_socket_create(NET_PROT_TCP);
  if (cl_main.conn.socket_tcp == NET_INVALID_SOCKET) {
    fprintf(stderr, "cl_init: failed to create TCP socket\n");
    net_socket_close(cl_main.conn.socket_udp);
    return 0;
  }

  cl_main.conn.state = CON_UNITIALISED;
  cl_main.conn.ticks_elapsed = 0;
  return 1;
}

uint8_t cl_connect(netaddr_t server_addr) {
  if (cl_main.conn.state != CON_UNITIALISED && cl_main.conn.state != CON_ZOMBIE) {
    fprintf(stderr, "cl_connect: already connecting/connected\n");
    return 0;
  }

  cl_server_addr = server_addr;

  if (net_socket_connect(cl_main.conn.socket_tcp, server_addr) < 0) {
    return 0;
  }

  cl_main.conn.state = CON_WAITING;
  return 1;
}

// Polls the TCP handshake socket for the server's join response.
static void cl_think_waiting(void) {
  byte buf[256];
  int32_t n = net_recv(cl_main.conn.socket_tcp, NULL, buf, sizeof(buf));

  if (n < 0) {
    fprintf(stderr, "cl_think: lost TCP connection during handshake\n");
    cl_main.conn.state = CON_ZOMBIE;
    return;
  }
  if (n == 0) return; // nothing yet, keep waiting

  // TODO: parse the real join response (serverinfo_t / assigned id) once
  // that part of protocol.h exists. For now: any reply == approved.
  printf("Handshake accepted..\n");
  cl_main.conn.state = CON_CONNECTED;
}

// Sends this tick's usercmd and drains incoming snapshot packets.
// Raw struct blit over UDP for now — no sequencing/acks/loss handling.
// That's netchan_t's job once it exists; this just proves the pipe works.
static void cl_think_connected(void) {
  net_send(cl_main.conn.socket_udp, &cl_server_addr,
            (const byte *)&cl_main.conn.cmd, sizeof(cl_main.conn.cmd));

  byte buf[NET_MAX_PACKET];
  netaddr_t from;
  int32_t n = net_recv(cl_main.conn.socket_udp, &from, buf, sizeof(buf));

  if (n < 0) {
    fprintf(stderr, "cl_think: UDP recv error\n");
    return;
  }
  if (n == 0) return; // nothing this tick

  if (!net_addr_eq(&from, &cl_server_addr)) {
    return; // stray/spoofed packet, ignore
  }

  // TODO: hand buf/n to snapshot parsing once protocol.h defines the wire format.
  cl_main.conn.state = CON_ACTIVE;
}

void cl_think(void) {
  cl_main.conn.ticks_elapsed++;

  switch (cl_main.conn.state) {
    case CON_WAITING:
      cl_think_waiting();
      break;
    case CON_CONNECTED:
      update_cmd(&cl_main.conn.cmdbuffer.cmds[0]);
    case CON_ACTIVE:
      cl_think_connected();
      break;
    case CON_UNITIALISED:
    case CON_ZOMBIE:
    default:
      break;
  }
}


#define KEY_DOWN(key) (_input_key_down(cl_app->input, key))
#define KEY_PRESS(key) (_input_key_prss(cl_app->input, key))
#define KEY_RELEASE(key) (_input_key_release(cl_app->input, key))

#define MOUSE_DOWN(button) (_input_mouse_down(cl_app->input, button))
#define MOUSE_PRESS(button) (_input_mouse_press(cl_app->input, button))
#define MOUSE_RELEASE(button) (_input_mouse_release(cl_app->input, button))


static void update_cmd(usercmd_t* cmd){
  if (KEY_DOWN(SDL_SCANCODE_W)) cmd->mv_forward += 1;
  if (KEY_DOWN(SDL_SCANCODE_S)) cmd->mv_forward += -1;

  if (KEY_DOWN(SDL_SCANCODE_A)) cmd->mv_side += -1;
  if (KEY_DOWN(SDL_SCANCODE_D)) cmd->mv_side += 1;

  if (KEY_DOWN(SDL_SCANCODE_SPACE)) cmd->mv_up += 1;
  if (KEY_DOWN(SDL_SCANCODE_LCTRL)) cmd->mv_up += -1;

  //VectorCopy(cl_app->camera.angles, cmd->viewangles);
}

