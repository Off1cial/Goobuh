#pragma once

// Strictly vulkan
#include <SDL3/SDL.h>
#include <string>


namespace Plat
{
  class Input;
  class Window 
  {
    public:
      Window(const char* name, int width, int height);
      ~Window();
      void Shutdown();
      void PollEvents(Input& input);

      void GetDimensions(int& w, int& h) const {w = m_width; h = m_height;}
      void SetDimensions(int w, int h);

      bool ShouldClose() const {return m_quitsignal;}


      SDL_Window* GetSDLWindow() const {return m_window;}
      SDL_Renderer* GetSDLRenderer() const {return m_renderer;}

    private:
      int m_width;
      int m_height;

      bool m_quitsignal;

      static inline constexpr int MIN_WIDTH = 640;
      static inline constexpr int MIN_HEIGHT = 480;

      SDL_Window* m_window = nullptr;
      SDL_Renderer* m_renderer = nullptr;// Temporary for getting the app working 
      std::string name;
  };
}
