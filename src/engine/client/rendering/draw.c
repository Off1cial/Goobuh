#include "engine/client/rendering/renderer.h"



#include "engine/system/window.h"
#include <SDL3/SDL_video.h>
#include <glad/glad.h>

void R_Start( void )
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}


void R_Present( void )
{

    SDL_GL_SwapWindow( gSys_window->window );
}
