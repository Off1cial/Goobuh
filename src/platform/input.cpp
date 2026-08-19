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
  const bool* keys = SDL_GetKeyboardState(NULL);
  std::copy(keys, keys + SDL_SCANCODE_COUNT, mKeysCurrent.begin());
  mMouseCurrent = (mMouseLocked) ?
    SDL_GetRelativeMouseState(&mMouseDx, &mMouseDy) :
    SDL_GetMouseState(&mMouseX, &mMouseY);

  if (keys[SDL_SCANCODE_W]){
    printf("W\n");
  }else{
    printf("Not W\n");
  }
};


void Input::ProcessEvent(SDL_Event& event)
{

}
