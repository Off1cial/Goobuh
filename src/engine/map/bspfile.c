#include "engine/map/bsp.h"

#include "engine/map/map.h"


// Reading and writing to/from BSP Files

bool MAP_LoadBSP( const char *path )
{
  if (!path) return false;


  FILE *bsp_file = fopen( path, "rb" );
  if (!bsp_file)
  {
    Con_printf("Failed to load BSP, missing file: %s\n", path );
    return false;
  }
  
}