#include "platform/input.h"
#include "common//logsys.h"
#include <stdlib.h>
#include <stdio.h>


plt_input* platform_createinput(void)
{
  plt_input* new_state = malloc(sizeof(plt_input));
  if (!new_state){
    LOG_FATAL("Failed to allocate input state");
    return NULL;
  }
  memset(new_state, 0, sizeof(plt_input));
  return new_state;
}


void platform_destroyinput(plt_input* state)
{
  free(state);
}

void input_update(plt_input* state)
{
  memcpy(
       state->keys_prev.keys, 
       state->keys_current.keys, 
       SDL_SCANCODE_COUNT);
    
  const bool* new_keys = SDL_GetKeyboardState(NULL);
  memcpy(state->keys_current.keys, new_keys, SDL_SCANCODE_COUNT);

  state->mouse_prev = state->mouse_current; 

  state->mouse_locked = 1;
  state->mouse_current = (state->mouse_locked)
    ? SDL_GetRelativeMouseState(&state->mx_rel, &state->my_rel)
    : SDL_GetMouseState(&state->mx, &state->my);
  //printf("mxrel, myrel = %0.2f, %0.2f\n", state->mx_rel, state->my_rel);

}

