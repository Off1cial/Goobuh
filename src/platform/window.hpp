#pragma once

// Strictly vulkan
#include <SDL3/SDL.h>

namespace Plat
{
  class Window 
  {
    public:
      Window() = default;
      Window(int width, int height) : m_width(width), m_height(height) {}
      
      void GetDimensions(int& w, int& h) const {w = m_width; h = m_height;}
      void SetDimensions(int w, int h);

    private:
      int m_width;
      int m_height;

      SDL_Window* m_window = nullptr;
  };
}
