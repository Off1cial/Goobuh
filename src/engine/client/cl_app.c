#include "engine/client/cl_app.h"
#include "common/logsys.h"
#include <stdlib.h>

struct clientapp_t* cl_app = NULL;

void app_init(const char* app_name, int winw, int winh)
{
  cl_app = calloc(1, sizeof(struct clientapp_t));
  if (!cl_app){
    LOG_FATAL("Failed to allocate global state");
  }  
  cl_app->window = platform_createwindow(app_name, winw, winh);
  cl_app->input = platform_createinput();
  cl_app->input->mouse_locked = 1;

  cl_app->renderer = malloc(sizeof(VK_Renderer));
  VK_Initialise(cl_app->renderer, cl_app->window->window);
  vec3_t cam_pos = {0, 0, 5.0f};
  camera_init(&cl_app->camera, cam_pos, AXIS_ZN, winw/ (float)winh, 60.0f);

}

static void app_pollevents(void)
{
  SDL_Event event;
  while (SDL_PollEvent(&event)){
    if (event.type == SDL_EVENT_QUIT){
      cl_app->window->should_close = 1; 
    }
    if (event.type == SDL_EVENT_WINDOW_RESIZED){
      cl_app->window->width = event.window.data1;
      cl_app->window->height = event.window.data2;
    }
  }
  input_update(cl_app->input);
}

static void window_resize(void){
  VK_WindowResize(cl_app->renderer);

  cl_app->camera.aspect = (cl_app->window->width / (float)cl_app->window->height);
}


void app_run(void)
{
  while (!cl_app->window->should_close){
    app_pollevents();
    if (cl_app->renderer->winresize_request){
      window_resize();
    }
    
    VK_Draw(cl_app->renderer, &cl_app->camera);
  }
  app_shutdown();
}


void app_shutdown(void)
{
  //Physics_Shutdown();
  VK_Shutdown(cl_app->renderer);
  platform_destroyinput(cl_app->input);
  platform_destroywindow(cl_app->window);
}
