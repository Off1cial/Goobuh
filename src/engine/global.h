#ifndef GLOBAL_H
#define GLOBAL_H

#include "platform/window.h"
#include "platform/input.h"
#include "renderer/vulkan/vk_renderer.h"

typedef struct GlobalState
{
  plt_window* window;
  plt_input* input;
  VK_Renderer* renderer;

  camera_t* camera_active;

} GlobalState;

extern GlobalState* g_Global;


void Global_Create(const char* AppName, int window_width, int window_height);
void Global_PollEvents(void);


void Global_Run(void);
void Global_Shutdown(void);

#endif // GLOBAL_H
