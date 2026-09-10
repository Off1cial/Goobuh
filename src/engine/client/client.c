#include "engine/client/client.h"
#include "engine/client/cl_app.h"

#define KEY_DOWN(key) (_input_key_down(cl_app->input, key))
#define KEY_PRESS(key) (_input_key_prss(cl_app->input, key))
#define KEY_RELEASE(key) (_input_key_release(cl_app->input, key))

#define MOUSE_DOWN(button) (_input_mouse_down(cl_app->input, button))
#define MOUSE_PRESS(button) (_input_mouse_press(cl_app->input, button))
#define MOUSE_RELEASE(button) (_input_mouse_release(cl_app->input, button))

static void update_cmd(usercmd_t* cmd){
  if (KEY_DOWN(SDL_SCANCODE_W)) cmd->mv_forward = 1;
  if (KEY_DOWN(SDL_SCANCODE_S)) cmd->mv_forward = -1;

  if (KEY_DOWN(SDL_SCANCODE_A)) cmd->mv_side = -1;
  if (KEY_DOWN(SDL_SCANCODE_D)) cmd->mv_side = 1;

  if (KEY_DOWN(SDL_SCANCODE_SPACE)) cmd->mv_up = 1;
  if (KEY_DOWN(SDL_SCANCODE_LCTRL)) cmd->mv_up = -1;

  VectorCopy(cl_app->camera.angles, cmd->viewangles);
}


void cl_think(void){
  // Check connection?

  // Update user command
  update_cmd(&cl_client.conn.cmd);
   

  // Send things?
}
