#include "renderer/vulkan/renderer.hpp"
#include "renderer/vulkan/vk_mesh.h"
#include "renderer/vulkan/vk_types.h"
#include "assetmanager.hpp"
#include "math/matrix.h"
#include "camera.h"

#include <cstring>

// Base push constants; copied per draw so each model gets its own transform.
static PushConstants s_base_push_consts = {
    .projection = _MatIdentity_,
    .view = _MatIdentity_,
    .model = _MatIdentity_,
    .vertex_addr = 0
};

CRendererVK::CRendererVK( SDL_Window* window ) : m_window( window )
{
    vk_renderer = new VK_Renderer();
    VK_Initialise( vk_renderer, window );
}

CRendererVK::~CRendererVK()
{
    // TODO: call your Vulkan shutdown function here (device/swapchain teardown).
    // Any loaded assets must be unloaded BEFORE this runs, since their VKMeshes
    // are GPU resources owned by this device.
    delete vk_renderer;
    vk_renderer = nullptr;
}

void CRendererVK::ResizeWindow( void )
{
    vk_renderer->winresize_request = true;
}

void CRendererVK::StartRendering( camera_t* camera )
{
    // TODO: if VK_BeginRendering doesn't already do it, fill the view/projection
    // in s_base_push_consts (or a per-frame copy) from `camera` here.
    MatrixCopy( camera->view, s_base_push_consts.view );
    MatrixCopy( camera->proj, s_base_push_consts.projection );
    VK_BeginRendering( vk_renderer, camera );
}

void CRendererVK::EndRendering( void )
{
    VK_EndRendering( vk_renderer );
}

bool CRendererVK::LoadModel( AssetHandle handle )
{
    return g_AssetManager->Load( this, handle );
}

VKMesh* CRendererVK::GetModel( AssetHandle handle )
{
    Asset* asset = g_AssetManager->Get( handle );
    if (!asset || asset->state != AssetState::Loaded)
        return nullptr;

    ModelData* mdl = asset->Model();
    return mdl ? &mdl->vk_mesh : nullptr;
}

void CRendererVK::DrawEntity( vec3_t origin, qangle angles, AssetHandle model )
{
    mat4 transform;
    MatrixModel( origin, angles, transform );
    DrawModel( model, transform );
}

void CRendererVK::DrawModel( AssetHandle model, mat4 transform )
{
    // Not loaded (or not a model): skip. Load assets outside the render pass with LoadModel().
    VKMesh* mesh = GetModel( model );
    if (!mesh){
        printf("Cant render unloaded model\n");
        return;
    }
        

    PushConstants push = s_base_push_consts;
    memcpy( &push.model, transform, sizeof( push.model ) ); // previously the transform was ignored

    VKMesh_draw(
        vk_renderer->device,
        vk_renderer->pipeline_layout,
        vk_renderer->cmd_active,
        vk_renderer->pipeline,
        mesh,   // the stored mesh, not a copy
        &push
    );
}