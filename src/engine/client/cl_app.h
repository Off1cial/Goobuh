#pragma once

#include "renderer/vulkan/vk_renderer.h"
#include "platform/window.h"
#include "platform/input.h"
#include "engine/client/camera.h"

struct clientapp_t{
  plt_window* window;
  plt_input* input;
  VK_Renderer* renderer;
  camera_t camera;
};

extern struct clientapp_t* cl_app;

void app_init(const char* app_name, int winw, int winh);
void app_run(void);
void app_shutdown(void);
