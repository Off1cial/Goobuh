#pragma once

#include "entity.h"
#include "renderer/vulkan/vk_renderer.h"
#include "math/matrix.h"
#include "public/engine/assethandle.hpp"

// Forward declarations
typedef struct VKMesh VKMesh;
typedef struct camera_t camera_t;
typedef struct entity_state_t entity_state_t;


class IRenderer
{
public:
    virtual ~IRenderer() = default;

    virtual void DrawModel( AssetHandle model, mat4 transform, uint32_t texture_index ) = 0;
    virtual void DrawEntity( entity_state_t state ) = 0;
    virtual void ResizeWindow( void ) = 0;

    virtual void StartRendering( camera_t* camera ) = 0;
    virtual void EndRendering( void ) = 0;

    // Owned by the concrete renderer; the asset manager reads it when loading meshes.
    VK_Renderer* vk_renderer = nullptr;
};
