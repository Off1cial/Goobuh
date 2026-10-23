#include "engine/client/client.h"

#include "engine/client/cl_input.h"
#include "engine/rendering/renderer.h"

clientstate_t cl = {0};

void CL_Init( void )
{
}

void CL_Frame( void )
{
    IN_Update();

    R_Start( cl.viewangles );

    R_Present();

}


