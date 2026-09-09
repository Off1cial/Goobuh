#include "platform/window.h"
#include "common/logsys.h"
#include <stdlib.h>
#include <SDL3/SDL_vulkan.h>

AppWindow* Platform_CreateWindow(
    const char* name,
    int width, int height
    )
{
  SDL_Window* window = 
    SDL_CreateWindow(name, width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE); 

  if (!window)
  {
    LOG_FATAL("Failed to create SDL Window");
    return NULL;
  }

  AppWindow* win = malloc(sizeof(AppWindow));
  if (!win){
    LOG_FATAL("Failed to allocate memory to app window");
    return NULL;
  }

  win->width = width;
  win->height = height;
  win->should_close = 0;
  win->window = window;

  SDL_SetWindowRelativeMouseMode(window, true);
  return win;
}


void Platform_DestroyWindow(AppWindow* window)
{
  if (!window)
    return;

  if (window->window)SDL_DestroyWindow(window->window);
  SDL_Vulkan_UnloadLibrary();

  free(window);
}
