#ifndef WINDOW_H
#define WINDOW_H



#include <SDL3/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct plt_window
{
  SDL_Window* window;
  int width, height;
  bool should_close;
} plt_window;

plt_window* platform_createwindow(
    const char* name,
    int width, int height
    );

void platform_destroywindow(plt_window* window);


#ifdef __cplusplus
}
#endif

#endif

