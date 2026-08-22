#include "engine/global.hpp"
#include "core/logsys.hpp"
#include <memory>

Global::Global()
{
  Log_Init("logfile.log");
  m_window = std::make_unique<Plat::Window>("Engine",640, 480);
  m_vkrenderer = std::make_unique<VK::Renderer>(*m_window);
  m_input = std::make_unique<Plat::Input>(m_window->GetSDLWindow());

}

void Global::Shutdown()
{
  m_window->Shutdown();
  Log_Shutdown();
  SDL_Quit();
}
Global::~Global()
{
  Shutdown();
}

void Global::Run()
{
  while (!m_window->ShouldClose())
  {
    m_window->PollEvents(*m_input);
    m_input->FrameStart();


    //m_vkrenderer->FrameStart();

    m_vkrenderer->Draw();
    //m_vkrenderer->FrameEnd();
  }
}
