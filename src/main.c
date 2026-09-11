#include "engine/client/cl_app.h"

int main(void){
  app_init("Client", 1280, 720);

  app_run();
  app_shutdown();
  return 0;
}
