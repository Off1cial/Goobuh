#include "renderer/opengl/gl_engine.hpp"
#include "core/logsys.hpp"
#include <iostream>

void DrawMesh(GL::MeshData& data, GLenum draw_mode) { GL:DrawMesh(data, draw_mode);};


void GLRenderer::Draw()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(0.0f, 0.3f, 0.7f, 1.0f);


  SDL_GL_SwapWindow(_sdl_window);
}

void GLRenderer::Shutdown()
{
  printf("Shutting down..");
}
//void GLRenderer::SubmitMesh(){}