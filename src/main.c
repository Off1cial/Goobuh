#include <stdio.h>

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
