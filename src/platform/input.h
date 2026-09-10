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

typedef struct plt_input
{
  struct keyboardstate_t keys_prev;
  struct keyboardstate_t keys_current;

  SDL_MouseButtonFlags mouse_prev;
  SDL_MouseButtonFlags mouse_current;
  uint8_t mouse_locked;
  
  float mx, my;
  float mx_rel, my_rel;

} plt_input;

static inline uint8_t _input_key_down(plt_input* state, SDL_Scancode key){
  return state->keys_current.keys[key];
}

static inline uint8_t _input_key_press(plt_input* state, SDL_Scancode key){
  return state->keys_current.keys[key] && !state->keys_prev.keys[key];
}

static inline uint8_t _input_key_release(plt_input* state, SDL_Scancode key){
  return !state->keys_current.keys[key] && state->keys_prev.keys[key];
}

static inline uint8_t _input_mouse_down(plt_input* state, mbutton_t button){
  return (state->mouse_current & button) != 0;
}

static inline uint8_t _input_mouse_press(plt_input* state, mbutton_t button ){
  return ( (state->mouse_current & button) != 0 )&& ( (state->mouse_prev & button) == 0);
}

static inline uint8_t _input_mouse_release(plt_input* state, mbutton_t button){
  return ( (state->mouse_current & button) == 0 )&& ( (state->mouse_prev & button) != 0);
}

plt_input* platform_createinput(void);
void platform_destroyinput(plt_input* state);
void input_update(plt_input* state);


#endif
