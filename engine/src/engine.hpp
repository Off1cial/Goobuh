#pragma once

#include "public/engine/game.hpp"
#include "public/engine/engine.hpp"
//#include "platform/input.h"
#include "input.hpp"
#include "platform/plt_time.h"
#include "platform/window.h"
#include "renderer/vulkan/vk_renderer.h"

#include "renderer/vulkan/renderer.hpp"

#include "net/networkmanager.h"

class Engine : public IEngine {
    public:
        Engine();
        ~Engine() = default;

        void Run();

        void LoadGame(IGame* game) { // Change this to take in a path, or make a separate class for the game interface
            m_game = game;
        }
    private:
        void Poll();

        void UpdateEntites( void );
        void DrawEntities( void );


        IGame* m_game = nullptr;
        plt_window* m_window = nullptr;
        CInput* m_input = nullptr;
        //VK_Renderer* m_renderer = nullptr;
        CRendererVK* m_vkrenderer = nullptr;
        bool m_quit = false;

        double m_frametime = 0.0; // Frametime in seconds
};