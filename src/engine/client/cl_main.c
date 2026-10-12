#include "engine/client/client.h"

#include "engine/client/cl_input.h"
#include "engine/client/rendering/renderer.h"

void CL_Init( void )
{
    

}

void CL_Frame( void )
{
    KeyUpdate();

    R_Start();

    R_Present();

}


