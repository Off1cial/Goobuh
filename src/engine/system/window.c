#include "engine/system/window.h"
#include "engine/system/allocator.h"
#include "engine/system/message.h"

#include "common/common.h"

#include <SDL3/SDL.h>
#include <glad/glad.h>
#include <stdio.h>

syswindow_t* gSys_window = NULL;

// Provide a pointer to the engine arena, allocates the window
bool Sys_WindowCreate( sysarena_t* engine_arena )
{

    gSys_window = Sys_ArenaAlloc( engine_arena, sizeof(syswindow_t) );
    if (!gSys_window){
        Sys_Crash( "Failed to allocate the application window" );
        return false;
    }
    if (!SDL_Init( SDL_INIT_VIDEO ))
    {
        Sys_Crash( "Failed to initialise SDL" );
        return false;
    }

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

    gSys_window->width = 640;
    gSys_window->height = 480;


    gSys_window->window = SDL_CreateWindow( "Client", WIN_WIDTH, WIN_HEIGHT, SDL_WINDOW_OPENGL );

    if (!gSys_window->window){
        Sys_Crash( "Failed to create the SDL window" );
        return false;
    }

   
    gSys_window->glContext = SDL_GL_CreateContext( gSys_window->window );
    if (!gSys_window->glContext)
    {
        Sys_Crash( "Failed to create the GL context" );
        return false;
    }

    if (!gladLoadGLLoader( (GLADloadproc)SDL_GL_GetProcAddress ))
    {
        Sys_Crash( "Glad GLL loader failed" );
        return false;
    }
    printf( "OpenGL version: %s\n", glGetString( GL_VERSION ) );
    printf( "GLSL version: %s\n", glGetString( GL_SHADING_LANGUAGE_VERSION ) );
    return true;
}



void Sys_WindowDestroy( void )
{
    SDL_GL_DestroyContext( gSys_window->glContext );
    SDL_DestroyWindow (gSys_window->window );
}
