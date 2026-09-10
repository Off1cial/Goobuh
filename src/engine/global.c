#include "engine/global.h"
#include "engine/player/player.h"
#include "common/logsys.h"
#include "engine/client/camera.h"
#include "renderer/vulkan/vk_renderer.h"


#include <SDL3/SDL.h>
#include <stdlib.h>
#include <stdio.h>

GlobalState* g_Global = NULL;


void Global_Create(const char* AppName, int window_width, int window_height)
{
  g_Global = malloc(sizeof(GlobalState));
  if (!g_Global){
    LOG_FATAL("Failed to allocate global state");
  }
  memset(g_Global, 0, sizeof(GlobalState));
  
  g_Global->window = platform_createwindow(AppName, window_width, window_height);
  g_Global->input = platform_createinput();
  g_Global->input->mouse_locked = 1;

  g_Global->renderer = malloc(sizeof(VK_Renderer));
  VK_Initialise(g_Global->renderer, g_Global->window->window);

  g_Global->camera_active = malloc(sizeof(camera_t));
  vec3_t cam_pos = {0, 0, 3.0f};
  camera_init(g_Global->camera_active, cam_pos, AXIS_ZN, ((float)window_width/(float)window_height), 90.0);


  //Physics_Init();
}

void Global_PollEvents(void)
{
  SDL_Event event;
  while (SDL_PollEvent(&event)){
    if (event.type == SDL_EVENT_QUIT){
      g_Global->window->should_close = 1; 
    }
    if (event.type == SDL_EVENT_WINDOW_RESIZED){
      g_Global->window->width = event.window.data1;
      g_Global->window->height = event.window.data2;
    }
  }
  input_update(g_Global->input);
}

static void window_resize(void){
  VK_WindowResize(g_Global->renderer);

  g_Global->camera_active->aspect = (g_Global->window->width / (float)g_Global->window->height);
}


void Global_Run(void)
{
  while (!g_Global->window->should_close){
    Global_PollEvents();
    if (g_Global->renderer->winresize_request){
      window_resize();
    }
    //printf("Player origin =  (%0.2f, %0.2f, %0.2f)\n", g_player.origin[0], g_player.origin[1], g_player.origin[2]);
    if (g_Global->camera_active){
      /*
      camera_look(
          g_Global->camera_active, 
          g_Global->input->mx_rel, 
          g_Global->input->my_rel, 
          0.02f);
      camera_update(g_Global->camera_active);
      */

      VK_Draw(g_Global->renderer, g_Global->camera_active);
    }
  }
  SDL_Delay(16);
  Global_Shutdown();
}


void Global_Shutdown(void)
{
  //Physics_Shutdown();
  VK_Shutdown(g_Global->renderer);
  platform_destroyinput(g_Global->input);
  platform_destroywindow(g_Global->window);
}
