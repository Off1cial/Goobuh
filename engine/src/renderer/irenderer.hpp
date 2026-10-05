#pragma once

#include "renderer/vulkan/vk_renderer.h"
#include "math/matrix.h"
#include "public/engine/assethandle.hpp"

// Forward declarations
typedef struct VKMesh VKMesh;
typedef struct camera_t camera_t;

class IRenderer
{
public:
    virtual ~IRenderer() = default;

    virtual void DrawModel( AssetHandle model, mat4 transform ) = 0;
    virtual void DrawEntity( vec3_t origin, qangle angles, AssetHandle model ) = 0;
    virtual void ResizeWindow( void ) = 0;

    virtual void StartRendering( camera_t* camera ) = 0;
    virtual void EndRendering( void ) = 0;

    // Owned by the concrete renderer; the asset manager reads it when loading meshes.
    VK_Renderer* vk_renderer = nullptr;
};