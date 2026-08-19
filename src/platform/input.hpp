#ifndef INPUT_H
#define INPUT_H

#include "platform/event.h"
#include "core/common.h"

#include <SDL3/SDL.h>
#include <array>

typedef enum {
  MOUSE_LEFT = SDL_BUTTON_LEFT,
  MOUSE_RIGHT = SDL_BUTTON_RIGHT,
  MOUSE_MIDDLE = SDL_BUTTON_MIDDLE,
  MOUSE_FRONT = SDL_BUTTON_X1,
  MOUSE_BACK = SDL_BUTTON_X2,
  MOUSE_BUTTON_COUNT = 5
} mbutton_t; // Does not support weird people with a whole keyboard on their mouse

/*
typedef struct CInput_t CInput;

struct CInput_t
{
  bool keys_current[SDL_SCANCODE_COUNT];
  bool keys_prev[SDL_SCANCODE_COUNT];
  
  SDL_MouseButtonFlags mouse_current;
  SDL_MouseButtonFlags mouse_prev;
  bool mouse_locked;
  float mx, my;
  float mxrel, myrel;

  void (*Shutdown)(CInput* self);
};

inline bool CInput_Pressed(CInput* self, SDL_Scancode key);

inline bool CInput_Released(CInput* self, SDL_Scancode key);

inline bool CInput_Held(CInput* self, SDL_Scancode key);


CInput* CInput_Create( void );
bool CInput_Poll( CInput* input, event_t* event);

*/

class Camera;

class CInput
{
  public:

    CInput(SDL_Window* window);

    void GetMousePos(float& x, float& y) const {x = mMouseX; y = mMouseY;}
    void GetMouseDelta(float& x, float& y) const {x = mMouseDx; y = mMouseDy;}
    void SetMousePos(float x, float y);
    bool PollEvent(event_t& event);

    bool KeyDown(SDL_Scancode key) const {return mKeysCurrent[key];}
    bool KeyPress(SDL_Scancode key) const {return mKeysCurrent[key] && !mKeysPrev[key];}
    bool KeyReleased(SDL_Scancode key) const {return !mKeysCurrent[key] && mKeysPrev[key];}

    bool MouseDown(mbutton_t button) const {return (mMouseCurrent & button) != 0;}
    bool MouseClick(mbutton_t button) const {return (mMouseCurrent & button) != 0 && (mMousePrev & button) == 0;}
    bool MouseReleased(mbutton_t button) const {return (mMouseCurrent & button) == 0 && (mMousePrev & button) != 0;}


    void SetActiveSDLWindow(SDL_Window* window) {mSDLwindow = window;}
    void SetMouseLock(bool lock);

  private:
    std::array<u8, SDL_SCANCODE_COUNT> mKeysCurrent;
    std::array<u8, SDL_SCANCODE_COUNT> mKeysPrev;
    SDL_MouseButtonFlags                 mMouseCurrent;
    SDL_MouseButtonFlags                 mMousePrev;
    bool mMouseLocked = 1;
    float mMouseX = 0;
    float mMouseY = 0;
    float mMouseDx = 0;
    float mMouseDy = 0;

  protected:
    SDL_Window* mSDLwindow = nullptr;
};



#endif

