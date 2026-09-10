#include "engine/player/playercontroller.h"
#include "platform/input.h"
#include "engine/shared/playerdata.h"


void playercontroller_update(struct playercontroller_t *cont, InputState *input){
  if (!cont || !input) return;
  
  cont->movflags = 0;
  cont->forward = 0; cont->side = 0; cont->up = 0;

  if (input->keys_current.keys[SDL_SCANCODE_W])
    cont->forward = 1;
  if (input->keys_current.keys[SDL_SCANCODE_S])
    cont->forward = -1;

  if (input->keys_current.keys[SDL_SCANCODE_A])
    cont->side = -1;
  if (input->keys_current.keys[SDL_SCANCODE_D])
    cont->side = 1;

  if (input->keys_current.keys[SDL_SCANCODE_SPACE])
    cont->up = 1;
  if (input->keys_current.keys[SDL_SCANCODE_LCTRL])
    cont->up = -1;
  
  cont->cam_lookx = input->mx_rel * cont->cam_sens;
  cont->cam_looky = input->my_rel * cont->cam_sens;

}
