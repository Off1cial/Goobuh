#include "platform/input.hpp"
#include "core/logsys.hpp"

using namespace Plat;

Input::Input(SDL_Window* window) : mSDLwindow(window)
{
  mKeysCurrent.fill(0);
  mKeysPrev.fill(0);
  SetMouseLock(mMouseLocked);
}

void Input::SetMouseLock(bool lock)
{
  mMouseLocked = lock;
  SDL_SetWindowRelativeMouseMode(mSDLwindow, mMouseLocked);
  SDL_SetWindowMouseGrab(mSDLwindow, mMouseLocked);
}

void Input::FrameStart()
{
  mMousePrev = mMouseCurrent;
  mKeysPrev = mKeysCurrent;
};


void Input::ProcessEvent(SDL_Event& event)
{

  switch(event.type)
  {
    case SDL_EVENT_KEY_DOWN:
    {
      if (event.key.key < 0 || event.key.key >= SDL_SCANCODE_COUNT) break;
      mKeysCurrent[event.key.key] = 1;
      break;
    }

    case SDL_EVENT_KEY_UP:
    {
      if (event.key.key < 0 || event.key.key >= SDL_SCANCODE_COUNT) break;
      mKeysCurrent[event.key.key] = 0;
      break;
    }

    case SDL_EVENT_MOUSE_MOTION:
    {
      mMouseX = (float)event.motion.x;
      mMouseY = (float)event.motion.y;
      mMouseDx = (float)event.motion.xrel;
      mMouseDy = (float)event.motion.yrel;
      break;
    }

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    {
      mMouseCurrent |= event.button.button;
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP:
    {
      mMouseCurrent &= ~event.button.button;
      break;
    }
  }
}
