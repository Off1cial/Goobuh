#include "engine/global.h"
#include "common/logsys.h"
#include "renderer/camera.h"
#include "renderer/vulkan/vk_renderer.h"

#include <SDL3/SDL.h>
#include <stdlib.h>

GlobalState* g_Global = NULL;


void Global_Create(const char* AppName, int window_width, int window_height)
{
  g_Global = malloc(sizeof(GlobalState));
  if (!g_Global){
    LOG_FATAL("Failed to allocate global state");
  }
  memset(g_Global, 0, sizeof(GlobalState));
  
  g_Global->window = Platform_CreateWindow(AppName, window_width, window_height);
  g_Global->input = Platform_CreateInput();

  g_Global->renderer = malloc(sizeof(VK_Renderer));
  VK_Initialise(g_Global->renderer, g_Global->window->window);

  g_Global->camera_active = malloc(sizeof(camera_t));
  vec3_t cam_pos = {0, 0, 0.8f};
  camera_init(g_Global->camera_active, cam_pos, AXIS_ZN, ((float)window_width/(float)window_height), 60.0);
}

void Global_PollEvents(void)
{
  SDL_Event event;
  while (SDL_PollEvent(&event)){
    if (event.type == SDL_EVENT_QUIT){
      g_Global->window->should_close = 1; 
    }
  }
  input_update(g_Global->input);
}


void Global_Run(void)
{
  while (!g_Global->window->should_close){
    Global_PollEvents();
    input_update(g_Global->input);

    if (g_Global->camera_active){
      camera_look(
          g_Global->camera_active, 
          g_Global->input->mx_rel, 
          g_Global->input->my_rel, 
          0.02f);
      camera_update(g_Global->camera_active);

      VK_Draw(g_Global->renderer, g_Global->camera_active);
    }
  }
  Global_Shutdown();
}


void Global_Shutdown(void)
{
  VK_Shutdown(g_Global->renderer);
  Platform_DestroyInput(g_Global->input);
  Platform_DestroyWindow(g_Global->window);
}
