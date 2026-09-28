#pragma once

#include "public/engine/game.hpp"
#include "public/engine/engine.hpp"
#include "platform/input.h"
#include "platform/plt_time.h"
#include "platform/window.h"
#include "renderer/vulkan/vk_renderer.h"

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
        IGame* m_game = nullptr;
        plt_window* m_window = nullptr;
        plt_input* m_input = nullptr;
        VK_Renderer* m_renderer = nullptr;
        bool m_quit = false;

        double m_frametime = 0.0; // Frametime in seconds
};