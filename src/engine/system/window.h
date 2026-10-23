#ifndef SYS_WINDOW_H
#define SYS_WINDOW_H

#include "SDL3/SDL.h"
#include <glad/glad.h>

#define WIN_WIDTH (gSys_window->width)
#define WIN_HEIGHT (gSys_window->height)

typedef struct syswindow_t
{
    int width, height;
    float centre[2];

    SDL_Window* window;
    SDL_GLContext glContext;

    int should_close;
} syswindow_t;

extern syswindow_t* gSys_window;


// Forward delcaration
typedef struct sysarena_t sysarena_t;

// Provide a pointer to the engine arena, allocates the window
bool Sys_WindowCreate( sysarena_t* engine_arena );
void Sys_WindowResize( void );


void Sys_WindowDestroy( void );

#endif
