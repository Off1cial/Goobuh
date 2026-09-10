#include <stdio.h>

/*
#include "engine/global.h"
#include "renderer/vulkan/vk_renderer.h"

VK_Renderer* renderer = NULL;


// TODO SET UP PIPELINE TO TAKE MESH DATA

int main(void){

  Global_Create("Goobuh engine", 640, 480);

  printf("Hello world\n");
  
  Global_Run();
  Global_Shutdown();

  return 0;
}
*/

#include "engine/client/cl_app.h"

int main(void){
  app_init("Client", 640, 480);

  app_run();
  app_shutdown();
  return 0;
}
