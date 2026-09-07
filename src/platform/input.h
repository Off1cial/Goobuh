#ifndef INPUT_H
#define INPUT_H

#include <SDL3/SDL.h>

typedef enum mbutton_t
{
  MBUTTON_LEFT = SDL_BUTTON_LEFT,
  MBUTTON_RIGHT = SDL_BUTTON_RIGHT,
  MBUTTON_MIDDLE = SDL_BUTTON_MIDDLE,
} mbutton_t;

struct keyboardstate_t
{
  uint8_t keys[SDL_SCANCODE_COUNT]; 
};

typedef struct InputState
{
  struct keyboardstate_t keys_prev;
  struct keyboardstate_t keys_current;

  SDL_MouseButtonFlags mouse_prev;
  SDL_MouseButtonFlags mouse_current;
  uint8_t mouse_locked;
  
  float mx, my;
  float mx_rel, my_rel;

} InputState;


InputState* Platform_CreateInput(void);
void Platform_DestroyInput(InputState* state);
void input_update(InputState* state);


#endif
