#include "engine/global.hpp"
#include "renderer/vulkan/vk_engine.hpp"
#include "core/logsys.hpp"
#include <memory>

Global::Global()
{
  Log_Init("logfile.log");
  _window = std::make_unique<Plat::Window>("Engine",640, 480);
  _renderer = std::make_unique<VK::Engine>(_window.get());
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

  }
}
