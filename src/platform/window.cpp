#include "platform/window.hpp"
#include "platform/input.hpp"
#include "core/logsys.hpp"
#include "core/common.h"
using namespace Plat;

#include <SDL3/SDL_vulkan.h>

Window::Window(const char* name, int width, int height) : 
  m_width(width), m_height(height)
{
  if (!SDL_Init(SDL_INIT_VIDEO)){
    LOG_FATAL("Failed to initialise SDL");
  } 

  m_width = MAX(MIN_WIDTH, width);
  m_height = MAX(MIN_HEIGHT, height);

  // TODO: Max window dimensions + handle resizing before allowing it SDL_WINDOW_RESIZABLE
  m_window = SDL_CreateWindow(
      name, 
      m_width, m_height, 
      SDL_WINDOW_VULKAN);


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
  if (m_renderer){
    SDL_DestroyRenderer(m_renderer);
    m_renderer =nullptr;
  }

  if (m_window){
    SDL_DestroyWindow(m_window);
    m_window = nullptr;
  }
  SDL_Vulkan_UnloadLibrary();
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
