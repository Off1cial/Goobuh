#include "engine/client/client.h"
#include "engine/client/cl_app.h"

#define KEY_DOWN(key) (_input_key_down(cl_app->input, key))
#define KEY_PRESS(key) (_input_key_prss(cl_app->input, key))
#define KEY_RELEASE(key) (_input_key_release(cl_app->input, key))

#define MOUSE_DOWN(button) (_input_mouse_down(cl_app->input, button))
#define MOUSE_PRESS(button) (_input_mouse_press(cl_app->input, button))
#define MOUSE_RELEASE(button) (_input_mouse_release(cl_app->input, button))

float cl_updaterate = 20;
float cl_interp = 0.1f;

client_t cl_main = {0};

static void update_cmd(usercmd_t* cmd){
  if (KEY_DOWN(SDL_SCANCODE_W)) cmd->mv_forward += 1;
  if (KEY_DOWN(SDL_SCANCODE_S)) cmd->mv_forward += -1;

  if (KEY_DOWN(SDL_SCANCODE_A)) cmd->mv_side += -1;
  if (KEY_DOWN(SDL_SCANCODE_D)) cmd->mv_side += 1;

  if (KEY_DOWN(SDL_SCANCODE_SPACE)) cmd->mv_up += 1;
  if (KEY_DOWN(SDL_SCANCODE_LCTRL)) cmd->mv_up += -1;

  VectorCopy(cl_app->camera.angles, cmd->viewangles);
}

static inline int8_t cl_setaddr(char* ip, uint16_t port){
  if (cl_main.conn.socket_udp == -1){
    cl_main.conn.socket_udp = netsocket_create(NET_PROT_UDP);
  }
  cl_main.conn.chan.remote = netaddr_create(ip, port);
  /*
  return netsocket_bind(
      cl_main.conn.socket_udp, 
      cl_main.conn.chan.remote);  
  Dont bind yet?
*/
  return 1;
}




void cl_think(void){
  // Check connection?

  // Update user command
  printf("Think..\n");
  update_cmd(&cl_main.conn.cmd);
   

  // Send things?
}
