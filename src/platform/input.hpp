#ifndef INPUT_H
#define INPUT_H

#include "core/common.h"

#include <SDL3/SDL.h>
#include <array>

typedef enum
{
  MOUSE_LEFT = SDL_BUTTON_LEFT,
  MOUSE_RIGHT = SDL_BUTTON_RIGHT,
  MOUSE_MIDDLE = SDL_BUTTON_MIDDLE,
  MOUSE_FRONT = SDL_BUTTON_X1,
  MOUSE_BACK = SDL_BUTTON_X2,
  MOUSE_BUTTON_COUNT = 5
} mbutton_t; // Does not support weird people with a whole keyboard on their mouse

class Camera;

namespace Plat
{
  class Input
  {
  public:
    Input(SDL_Window *window);

    void GetMousePos(float &x, float &y) const
    {
      x = mMouseX;
      y = mMouseY;
    }
    void GetMouseDelta(float &x, float &y) const
    {
      x = mMouseDx;
      y = mMouseDy;
    }
    void SetMousePos(float x, float y);

    void FrameStart();
    void ProcessEvent(SDL_Event& event);

    bool KeyDown(SDL_Scancode key) const { return mKeysCurrent[key]; }
    bool KeyPress(SDL_Scancode key) const { return mKeysCurrent[key] && !mKeysPrev[key]; }
    bool KeyReleased(SDL_Scancode key) const { return !mKeysCurrent[key] && mKeysPrev[key]; }

    bool MouseDown(mbutton_t button) const { return (mMouseCurrent & button) != 0; }
    bool MouseClick(mbutton_t button) const { return (mMouseCurrent & button) != 0 && (mMousePrev & button) == 0; }
    bool MouseReleased(mbutton_t button) const { return (mMouseCurrent & button) == 0 && (mMousePrev & button) != 0; }

    void SetActiveSDLWindow(SDL_Window *window) { mSDLwindow = window; }
    void SetMouseLock(bool lock);

  private:
    std::array<u8, SDL_SCANCODE_COUNT> mKeysCurrent;
    std::array<u8, SDL_SCANCODE_COUNT> mKeysPrev;
    SDL_MouseButtonFlags mMouseCurrent;
    SDL_MouseButtonFlags mMousePrev;
    bool mMouseLocked = 1;
    float mMouseX = 0;
    float mMouseY = 0;
    float mMouseDx = 0;
    float mMouseDy = 0;

  protected:
    SDL_Window *mSDLwindow = nullptr;
  };
}

#endif
