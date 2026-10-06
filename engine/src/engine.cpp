#include "engine.hpp"
#include "platform/input.h"
#include "platform/plt_time.h"
#include "platform/window.h"
#include "renderer/vulkan/vk_renderer.h"
#include "physics/physmanager.hpp"

#include "assetmanager.hpp"
#include "entity.h"


#include <SDL3/SDL.h>
#include "camera.h"

AssetHandle cone_model;
AssetHandle monkey_model;
AssetHandle testmap_model;
AssetHandle cube_model;

Engine::Engine() {
    m_window = platform_createwindow("G00BUH", 800, 600);
    //m_input = platform_createinput();
    m_input = new CInput();
    m_vkrenderer = new CRendererVK( m_window->window );
    //m_renderer = new VK_Renderer();
    //VK_Initialise(m_renderer, m_window->window); 
    g_AssetManager->Init({ASSET_DIR "/models", ASSET_DIR "/levels"});
    

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        g_Entities[i].free = true;
    }

    testmap_model = g_AssetManager->GetHandle("test.glb");
    cone_model = g_AssetManager->GetHandle("cone.glb");
    monkey_model = g_AssetManager->GetHandle("monkey.glb");
    cube_model = g_AssetManager->GetHandle("cube.glb");
    m_vkrenderer->LoadModel(cone_model);
    m_vkrenderer->LoadModel(monkey_model);
    m_vkrenderer->LoadModel(testmap_model);
    m_vkrenderer->LoadModel(cube_model);

    //entity_t* ent_world = ED_NEW( VEC_ZERO, VEC_ZERO, VEC_ZERO, VEC_HALF, testmap_model, 1, false, 0.0f );

    entity_t* ent_cube = ED_NEW( VEC_ZERO, (vec3_t){0.0f, 0.001f, -5.5f}, VEC_ZERO, VEC_HALF, cube_model, 0, true, 1.0f );
}

void Engine::Poll() {
    // Poll for events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            m_window->should_close = true;
        }
        // Handle other events (keyboard, mouse, etc.)
    }
    m_input->Poll();
}





double now = 0.0;
double previous = 0.0;
vec3_t pos = { 0,1,0 };
qangle angles = { 0.2,0.45,0.1 };




void Engine::Run() {

    camera_t camera;
    camera_init(&camera, (vec3_t){0.0f, 0.0f, 5.0f}, (vec3_t){0.0f, 0.0f, -1.0f}, 800.0f / 600.0f, 60.0);
    m_input->SetCamera(&camera);

    while (!m_window->should_close) {
        double now = plt_timemillis();
        m_frametime = (now - previous) / 1000.0; // Convert to seconds
        previous = now;

        g_NetworkManager->Update();
        Poll();


        if (m_input->KeyDown(KEY_W)){
            m_input->MoveCamera(camera.front, 0.1f);
        }

        // Do stuff
        m_game->update(m_frametime);
        m_input->AimCamera();
        camera_update(&camera);
        m_vkrenderer->StartRendering( &camera );
        mat4 testm;
        angles[YAW] += 0.01f;
        angles[PITCH] += 0.2;
        MatrixModel(pos, angles, testm);
        //m_vkrenderer->DrawModel( cone_model, testm, 0 );
        g_PhysicsManager->Simulate(  0.01f );
        UpdateEntites();
        DrawEntities();


        m_vkrenderer->EndRendering();

        //VK_Draw(m_renderer, &camera);
        SDL_Delay(8); // Simulate a frame delay (for demonstration purposes)
    }
}

void Engine::UpdateEntites( void )
{
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entity_t* e = &g_Entities[i];
        if (e->free) continue;
        if (!e->simulated) continue;
        g_PhysicsManager->GetObjectPosition( e->physobj_id, e->state.origin );
    }
}

void Engine::DrawEntities( void )
{
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entity_t* e = &g_Entities[i];
        if (e->free) continue;
        m_vkrenderer->DrawEntity( e->state );
       
    }
}
