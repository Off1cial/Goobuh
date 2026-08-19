#include "platform/input.hpp"
#include "core/logsys.h"

CInput::CInput(SDL_Window* window) : mSDLwindow(window)
{
  mKeysCurrent.fill(0);
  mKeysPrev.fill(0);
  SetMouseLock(mMouseLocked);
}

void CInput::SetMouseLock(bool lock)
{
  mMouseLocked = lock;
  SDL_SetWindowRelativeMouseMode(mSDLwindow, mMouseLocked);
  SDL_SetWindowMouseGrab(mSDLwindow, mMouseLocked);
}

bool CInput::PollEvent(event_t& event){

  mKeysPrev = mKeysCurrent;
  mMousePrev = mMouseCurrent;

  SDL_Event sdl_event;
  while (SDL_PollEvent(&sdl_event)){
    switch(sdl_event.type){
      case SDL_EVENT_QUIT:{
        event.type = EVENT_QUIT;
        return true;
      }
      case SDL_EVENT_WINDOW_RESIZED:{
        event.type = EVENT_WIN_RESIZE;
        event.win_resize.width = sdl_event.window.data1;
        event.win_resize.height = sdl_event.window.data2;
        return true;
      }
      case SDL_EVENT_KEY_DOWN:{
        if (sdl_event.key.key < 0 || sdl_event.key.key >= SDL_SCANCODE_COUNT) break;
        mKeysCurrent[sdl_event.key.key] = 1;
        break;
      }
      case SDL_EVENT_KEY_UP:
        if (sdl_event.key.key < 0 || sdl_event.key.key >= SDL_SCANCODE_COUNT) break;
        mKeysCurrent[sdl_event.key.key] = 0;
        break;
      default:
        break;
    }
  }
  
  if (mMouseLocked){
    mMouseCurrent = SDL_GetRelativeMouseState(&mMouseDx, &mMouseDy);
  }else{
    mMouseCurrent = SDL_GetGlobalMouseState(&mMouseX, &mMouseY);
  }
  

  return false;
}
