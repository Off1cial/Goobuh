#ifndef __RENDERERGL_H__
#define __RENDERERGL_H__

#include <SDL3/SDL.h>
#include <glad/glad.h>

namespace GL
{

class GLRenderSystem
{
public:
    void BeginFrame();
    void EndFrame();
private:
    SDL_Window* m_window; 
    SDL_GLContext m_context;
};
    
};
#endif
