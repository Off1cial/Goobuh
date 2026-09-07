#ifndef WINDOW_H
#define WINDOW_H

#include <SDL3/SDL.h>

typedef struct AppWindow
{
  SDL_Window* window;
  int width, height;
  bool should_close;
} AppWindow;

AppWindow* Platform_CreateWindow(
    const char* name,
    int width, int height
    );

void Platform_DestroyWindow(AppWindow* window);



#endif

