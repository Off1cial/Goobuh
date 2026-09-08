#include "platform/input.h"
#include "common//logsys.h"
#include <stdlib.h>
#include <stdio.h>


InputState* Platform_CreateInput(void)
{
  InputState* new_state = malloc(sizeof(InputState));
  if (!new_state){
    LOG_FATAL("Failed to allocate input state");
    return NULL;
  }
  memset(new_state, 0, sizeof(InputState));
  return new_state;
}


void Platform_DestroyInput(InputState* state)
{
  free(state);
}

void input_update(InputState* state)
{
  memcpy(
       state->keys_prev.keys, 
       state->keys_current.keys, 
       SDL_SCANCODE_COUNT);
    
  const bool* new_keys = SDL_GetKeyboardState(NULL);
  memcpy(state->keys_current.keys, new_keys, SDL_SCANCODE_COUNT);

  state->mouse_prev = state->mouse_current; 

  state->mouse_current = (state->mouse_locked)
    ? SDL_GetRelativeMouseState(&state->mx_rel, &state->my_rel)
    : SDL_GetMouseState(&state->mx, &state->my);

}

