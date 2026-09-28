#include "renderer/vkrenderer.hpp"


CRendererVK::CRendererVK( SDL_Window* window )
{
    renderer = new VK_Renderer();
    VK_Initialise( renderer, window );
}


