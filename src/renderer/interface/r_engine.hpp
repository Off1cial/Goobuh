#pragma once

// This is the interface abstraction of the renderer, allows the rest of the program to interact
// with the renderer in the same fashion, completely independent of the underlying API.

#include "platform/window.hpp"

class IRenderer
{
  public:
    virtual ~IRenderer() = default;
    virtual void Shutdown() = 0;

    virtual void Draw() = 0;
    //virtual void SubmitMesh() = 0;

    Plat::GraphicsAPI GetAPI() const { return _api; } 

  private:
    Plat::GraphicsAPI _api;
    virtual void Init() = 0;


};
