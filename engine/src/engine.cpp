#include "engine.hpp"
#include "common/common.h"
#include "math/vector.h"
#include "physics/backend/blockmanager.hpp"
#include "physics/physobject.hpp"
#include "platform/input.h"
#include "platform/plt_time.h"
#include "platform/window.h"
#include "renderer/vulkan/vk_renderer.h"
#include "physics/physmanager.hpp"

#include "soundmanager.hpp"

#include "assetmanager.hpp"
#include "entity.h"


#include <SDL3/SDL.h>
#include <simdjson.h>
#include "camera.h"

AssetHandle cone_model;
AssetHandle shell_model;
AssetHandle testmap_model;
AssetHandle cube_model;
AssetHandle metal_texture;

Engine::Engine() {

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO );

    m_window = platform_createwindow("G00BUH", 800, 600);
    //m_input = platform_createinput();
    m_input = new CInput();
    m_vkrenderer = new CRendererVK( m_window->window );
    //m_renderer = new VK_Renderer();
    //VK_Initialise(m_renderer, m_window->window); 
    g_AssetManager->Init({ASSET_DIR "/models", ASSET_DIR "/levels", ASSET_DIR "/textures"});

    g_NetworkManager = new CNetworkManager;
    g_NetworkManager->Init();





    for (int i = 0; i < MAX_ENTITIES; i++)
        g_Entities[i].free = true;

    testmap_model = g_AssetManager->GetHandle("test.glb");
    cone_model = g_AssetManager->GetHandle("cone.glb");
    shell_model = g_AssetManager->GetHandle("bomb.glb");
    cube_model = g_AssetManager->GetHandle("cube.glb");

    metal_texture = g_AssetManager->GetHandle("default.png");
    m_vkrenderer->LoadTexture(metal_texture);

    m_vkrenderer->LoadModel(cone_model);
    m_vkrenderer->LoadModel(shell_model);
    m_vkrenderer->LoadModel(testmap_model);
    m_vkrenderer->LoadModel(cube_model);

    //entity_t* ent_world = ED_NEW( VEC_ZERO, VEC_ZERO, VEC_ZERO, VEC_HALF, testmap_model, 1, false, 0.0f );
    vec3_t cube_size = {CM2UNITS( 100 ), CM2UNITS( 100 ), CM2UNITS( 100 ) };

    CEntity *ent_cube = SpawnEntity( VEC_ZERO, VEC_ZERO, VEC_ZERO, 55.0F, true, cube_model, metal_texture, cube_size );  // 1 m^2 squared
    CEntity* ent_cube1 = SpawnEntity( (vec3_t){0, 10, 0}, VEC_ZERO, VEC_ZERO , 40.0F, true, cube_model, metal_texture, cube_size );
    CEntity* ent_cube2 = SpawnEntity( (vec3_t) { 0, 10 + cube_size[2] * 5, 0 }, VEC_ZERO, VEC_ZERO, 40.0F, true, cube_model, metal_texture, cube_size );
    CEntity* ent_cube3 = SpawnEntity( (vec3_t) { 0, 15 + cube_size[2] * 10, 0 }, VEC_ZERO, VEC_ZERO, 40.0F, true, cube_model, metal_texture, cube_size );
    


    SDL_AudioSpec wav_spec{};
    Uint8* data = nullptr;
    Uint32 len = 0;

    if (SDL_LoadWAV(ASSET_DIR "/sounds/hitsound.wav",&wav_spec, &data, &len)) {

        if (g_SoundManager->Init(wav_spec)) {
            g_SoundManager->PlaySoundData(data, len, 0.1);
        }
        SDL_free(data);
    }
    else {
        SDL_Log("Failed to load WAV: %s", SDL_GetError());
    }
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
    camera_init(&camera, (vec3_t){0.0f, CM2UNITS(180), 400.0f}, (vec3_t){0.0f, 0.0f, -1.0f}, 800.0f / 600.0f, 90.0);
    m_input->SetCamera(&camera);

    CEntity* aabb_ent = SpawnEntity(
            camera.origin, VEC_ZERO, VEC_ZERO, 80.0f, true, cube_model, metal_texture, (vec3_t){CM2UNITS(30), CM2UNITS(180), CM2UNITS(30)}
            );
    g_PhysicsManager->EnableFlag( aabb_ent->GetPhysobj(), BODYSTATE_FLAG_ROTATION_LOCKED);
    physobjid_t physobjaabb = aabb_ent->GetPhysobj();
    while (!m_window->should_close) {
        double now = plt_timemillis();
        m_frametime = (now - previous) / 1000.0; // Convert to seconds
        previous = now;

        g_NetworkManager->Update();
        Poll();


        if (m_input->KeyDown(KEY_W)){
            m_input->MoveCamera(camera.front, CM2UNITS(10));
        }

        if (m_input->MouseClick( MOUSE_RIGHT ))
        {
            g_PhysicsManager->AddForceCentre( 0, VectorNew(0.0, 50, 0.0f ));
            vec3_t vel; VectorScale(camera.front, 800.0f, vel);
            CEntity* projectile = SpawnEntity( camera.origin, vel, camera.front, 5.0F, true, cube_model, metal_texture, (vec3_t){CM2UNITS(40), CM2UNITS( 40 ), CM2UNITS( 40 )} );
        }

        // Do stuff
        m_game->update(m_frametime);
        m_input->CameraLook();
        camera_update(&camera);
        m_vkrenderer->StartRendering( &camera );
        mat4 testm;
        angles[YAW] += 0.01f;
        angles[PITCH] += 0.2;
        MatrixModel(pos, angles, testm);
        //m_vkrenderer->DrawModel( cone_model, testm, 0 );
        g_PhysicsManager->Simulate(  0.006f );
        UpdateEntites();
        DrawEntities();


        m_vkrenderer->EndRendering();

        //VK_Draw(m_renderer, &camera);
        SDL_Delay(8);
    }
}

void Engine::UpdateEntites( void )
{
    for (CEntity& e : g_Entities)
    {
        if (e.free) continue;
        if (!e.HasPhysics()) continue;
        physobjid_t physobj = e.GetPhysobj();
        g_PhysicsManager->GetObjectPosition( physobj, e.state.origin );
        g_PhysicsManager->GetObjectRotation( physobj, e.state.rotation );

    }
}

void Engine::DrawEntities( void )
{
    for (CEntity& e : g_Entities)
    {
        if (e.free) continue;
        m_vkrenderer->DrawEntity(e.state);
    }
}
