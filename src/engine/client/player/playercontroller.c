#include "engine/client/player/playercontroller.h"
#include "platform/input.h"


void playercontroller_update(struct playercontroller_t *cont, InputState *input){
  if (!cont || !input) return;
  
  cont->movflags = 0;

  if (input->keys_current.keys[SDL_SCANCODE_W])
    cont->movflags |= PLAYERCONT_MOV_FWD;
  if (input->keys_current.keys[SDL_SCANCODE_S])
    cont->movflags |= PLAYERCONT_MOV_BACK;

  if (input->keys_current.keys[SDL_SCANCODE_A])
    cont->movflags |= PLAYERCONT_MOV_LEFT;
  if (input->keys_current.keys[SDL_SCANCODE_D])
    cont->movflags |= PLAYERCONT_MOV_RIGHT;

  if (input->keys_current.keys[SDL_SCANCODE_SPACE])
    cont->movflags |= PLAYERCONT_MOV_UP;
  if (input->keys_current.keys[SDL_SCANCODE_LCTRL])
    cont->movflags |= PLAYERCONT_MOV_DOWN;

  
  cont->cam_lookx = input->mx_rel * cont->cam_sens;
  cont->cam_looky = input->my_rel * cont->cam_sens;

}
