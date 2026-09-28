#pragma once


#include "renderer/irenderer.hpp"
#include "renderer/vulkan/vk_renderer.h"

#include <SDL3/SDL.h>

// Contains the old C-style struct
class CRendererVK : public IRenderer
{
public:
    CRendererVK( SDL_Window* window );
    void DrawModel( struct assethandle_t handle ) override;

    // Create a VKMesh by loading the glb specified by the handle
    VKMesh* GetModel( struct assethandle_t handle );

private:
    SDL_Window* window;
    VK_Renderer* renderer;
};