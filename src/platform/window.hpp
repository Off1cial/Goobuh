#pragma once

#include <SDL3/SDL.h>
#include <string>


namespace Plat
{
  class Input;

  enum class GraphicsAPI
  {
    OpenGL,
    Vulkan,
    DirectX
  };

  class Window 
  {
    public:
      Window(GraphicsAPI api, const char* name, int width, int height);
      ~Window();
      void Shutdown();
      void PollEvents(Input& input);

      std::string GetName() {return name;}

      void GetDimensions(int& w, int& h) const {w = m_width; h = m_height;}
      void SetDimensions(int w, int h);

      bool ShouldClose() const {return m_quitsignal;}


      SDL_Window* GetSDLWindow() const {return m_window;}

    private:
      int m_width;
      int m_height;

      bool m_quitsignal;

      static inline constexpr int MIN_WIDTH = 640;
      static inline constexpr int MIN_HEIGHT = 480;

      GraphicsAPI _api;

      SDL_Window* m_window = nullptr;
      std::string name;
  };
};
