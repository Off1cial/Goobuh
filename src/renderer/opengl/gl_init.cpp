#include "renderer/opengl/gl_engine.hpp"
#include "core/logsys.hpp"
#include <iostream>


void GLRenderer::Init()
{
  std::cout << "Initialising OpenGL renderer\n";
  _sdl_window = _window->GetSDLWindow();
  _context = SDL_GL_CreateContext(_window->GetSDLWindow());
  if (!_context || !gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    LOG_FATAL("Failed to create OpenGL context");

  printf("OpenGL: %s\n", glGetString(GL_VERSION));
  glEnable(GL_DEPTH_TEST);
}
