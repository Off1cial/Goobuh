#include "engine/global.hpp"
#include "renderer/vulkan/vk_engine.hpp"
#include "renderer/opengl/gl_engine.hpp"
#include "core/logsys.hpp"
#include <memory>

Global::Global(Plat::GraphicsAPI api)
{
  Log_Init("logfile.log");
  _window = std::make_unique<Plat::Window>(Plat::GraphicsAPI::OpenGL, "Engine",640, 480);
  if (api == Plat::GraphicsAPI::OpenGL){
    _renderer = std::make_unique<GLRenderer>(_window.get());
  }else{
    _renderer = std::make_unique<VK::VKRenderer>(_window.get());
  }
  _input = std::make_unique<Plat::Input>(_window->GetSDLWindow());

}

void Global::Shutdown()
{
  _window->Shutdown();
  Log_Shutdown();
  SDL_Quit();
}
Global::~Global()
{
  Shutdown();
}

void Global::Run()
{
  while (!_window->ShouldClose())
  {
    _window->PollEvents(*_input);
    _input->FrameStart();

    _renderer->Draw();

  }
}
