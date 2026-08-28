#include "platform/window.hpp"
#include "platform/input.hpp"
#include "core/logsys.hpp"
#include "core/common.h"
using namespace Plat;

#include <SDL3/SDL_vulkan.h>
Window::Window(GraphicsAPI api, const char* name, int width, int height) : 
  m_width(width), m_height(height), _api(api)
{
  if (!SDL_Init(SDL_INIT_VIDEO)){
    LOG_FATAL("Failed to initialise SDL");
  } 

  m_width = MAX(MIN_WIDTH, width);
  m_height = MAX(MIN_HEIGHT, height);

  // TODO: Max window dimensions + handle resizing before allowing it SDL_WINDOW_RESIZABLE
  uint64_t winflags = 0;
  if (api == GraphicsAPI::OpenGL){
    winflags = SDL_WINDOW_OPENGL;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  }else if (api == GraphicsAPI::Vulkan){
    winflags = SDL_WINDOW_VULKAN;
  }
  // Now add other window flags....
  // winflags |= SDL_WINDOW_RESIZABLE 
  m_window = SDL_CreateWindow(
      name, 
      m_width, m_height, 
      winflags);


  if (!m_window){
    LOG_FATAL("Failed to create SDL window");
  }
}

Window::~Window()
{
  Shutdown();
}

void Window::Shutdown()
{
  if (m_window){
    SDL_DestroyWindow(m_window);
    m_window = nullptr;
  }
  if (_api == GraphicsAPI::Vulkan)SDL_Vulkan_UnloadLibrary();
}

void Window::PollEvents(Input& input)
{
  SDL_Event event;
  while (SDL_PollEvent(&event)){
    switch(event.type){
      case SDL_EVENT_QUIT:{
        m_quitsignal = 1;
        break;
      }
    }
    input.ProcessEvent(event);
  }
}
