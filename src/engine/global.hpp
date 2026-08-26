#pragma once

#include "platform/input.hpp"
#include "platform/window.hpp"
#include <memory>
#include "renderer/vulkan/vk_engine.hpp"

class Global
{
  public:
    Global();
    ~Global();

    virtual void Shutdown();
    virtual void Run(); // Overrided by games e.g class MyGame : public Global, then void Run() override;


  private:
    std::unique_ptr<Plat::Window> _window = nullptr;
    std::unique_ptr<Plat::Input>  _input  = nullptr;
    std::unique_ptr<VK::Engine>   _renderer = nullptr;
};
