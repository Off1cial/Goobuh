#pragma once

#include "platform/input.hpp"
#include "platform/window.hpp"
#include "renderer/vulkan/vk_renderer.hpp"
#include <memory>

class Global
{
  public:
    Global();
    ~Global();

    virtual void Shutdown();
    virtual void Run(); // Overrided by games e.g class MyGame : public Global, then void Run() override;


  private:
    std::unique_ptr<Plat::Window> m_window = nullptr;
    std::unique_ptr<VK::Renderer> m_vkrenderer = nullptr;
    std::unique_ptr<Plat::Input>  m_input = nullptr;
};
