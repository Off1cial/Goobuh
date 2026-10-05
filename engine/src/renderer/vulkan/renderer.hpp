#pragma once

#include "renderer/irenderer.hpp"
#include "renderer/vulkan/vk_renderer.h"

#include "math/matrix.h"
#include "public/engine/assethandle.hpp"

#include <SDL3/SDL.h>

// Contains the old C-style struct
class CRendererVK : public IRenderer
{
public:
    explicit CRendererVK( SDL_Window* window );
    ~CRendererVK() override;

    CRendererVK( const CRendererVK& ) = delete;
    CRendererVK& operator=( const CRendererVK& ) = delete;

    void DrawModel( AssetHandle model, mat4 transform ) override;
    void DrawEntity( vec3_t origin, qangle angles, AssetHandle model ) override;

    void ResizeWindow( void ) override;

    void StartRendering( camera_t* camera ) override;
    void EndRendering( void ) override;

    // Loads the model's GPU data via the asset manager (call this outside StartRendering/EndRendering).
    bool LoadModel( AssetHandle handle );

    // Returns the GPU mesh for a handle, or nullptr if it isn't a loaded model. Never loads.
    VKMesh* GetModel( AssetHandle handle );

private:
    SDL_Window* m_window;
    // The VK_Renderer itself is IRenderer::vk_renderer (no second copy to keep in sync).
};