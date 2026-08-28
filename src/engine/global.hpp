#pragma once

#include "platform/input.hpp"
#include "platform/window.hpp"
#include "renderer/interface/r_engine.hpp"
#include <memory>

class Global
{
  public:
    Global(Plat::GraphicsAPI api);
    ~Global();

    virtual void Shutdown();
    virtual void Run(); // Overrided by games e.g class MyGame : public Global, then void Run() override;


  private:
    std::unique_ptr<Plat::Window> _window = nullptr;
    std::unique_ptr<Plat::Input>  _input  = nullptr;
    std::unique_ptr<IRenderer>   _renderer = nullptr;
};
