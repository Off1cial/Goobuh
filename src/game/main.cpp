#include "engine/global.hpp"


int main( void )
{
  Global gbl(Plat::GraphicsAPI::OpenGL);
  gbl.Run(); 
} 