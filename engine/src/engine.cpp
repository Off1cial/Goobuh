#include "engine.hpp"
#include "platform/input.h"
#include "platform/plt_time.h"
#include "platform/window.h"
#include "renderer/vulkan/vk_renderer.h"
#include <SDL3/SDL.h>
#include "camera.h"


Engine::Engine() {
    m_window = platform_createwindow("G00BUH", 800, 600);
    m_input = platform_createinput();
    m_renderer = new VK_Renderer();
    VK_Initialise(m_renderer, m_window->window);   
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
    input_update(m_input);
}

double now = 0.0;
double previous = 0.0;
void Engine::Run() {

    camera_t camera;
    camera_init(&camera, (vec3_t){0.0f, 0.0f, 5.0f}, (vec3_t){0.0f, 0.0f, -1.0f}, 800.0f / 600.0f, 60.0);

    while (!m_window->should_close) {
        double now = plt_timemillis();
        m_frametime = (now - previous) / 1000.0; // Convert to seconds
        previous = now;

        g_NetworkManager->Update();
        
        Poll();
        // Do stuff
        m_game->update(m_frametime);

        VK_Draw(m_renderer, &camera);
        SDL_Delay(16); // Simulate a frame delay (for demonstration purposes)
    }
}