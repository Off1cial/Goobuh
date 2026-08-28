#pragma once

#include "renderer/interface/r_engine.hpp"
#include "renderer/opengl/gl_types.h"
#include <glad/glad.h>

class GLRenderer : public IRenderer
{
  public:
    GLRenderer(Plat::Window* window) : _window(window), _sdl_window(window->GetSDLWindow()) {Init();}
    ~GLRenderer() {Shutdown();};

    void Shutdown() override;
    void Draw() override;

    void DrawMesh(GL::MeshData& data, GLenum draw_mode);

    //virtual void SubmitMesh() override;
    
  private:
    void Init() override;

    Plat::Window* _window;
    SDL_Window* _sdl_window;
    SDL_GLContext _context;

    

};
