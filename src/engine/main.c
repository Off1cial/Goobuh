#include <stdio.h>
#include <SDL3/SDL.h>

#include "common/common.h"
#include "engine/system/window.h"
#include "engine/system/allocator.h"

// Temporary
#include "math/matrix.h"
#include "rendering/renderer.h"

#include "engine.h"

#include "engine/net/net.h"
// test
#include "engine/client/client.h"
#include "engine/client/cl_input.h"

#include "engine/map/compile.h"
#include "engine/rendering/model.h"

sysarena_t LifeArena;
int main( void )
{

    

    Sys_ArenaInit( &LifeArena, HUNK_SIZE, "EngineHunk", MEMTAG_LIFE );

    engine_t* engine = Sys_ArenaAlloc( &LifeArena, sizeof(engine_t));
    Engine_LoadGameLib( engine );

    Sys_WindowCreate( &LifeArena );

    R_Init();
    model_t* test_mdl = MDL_Load("models/test.mdl");

    NET_Init();
    
    bool compiled = MAP_Compile( "maps/first.map", "maps/first.bsp" );
    if (!compiled)
    {
        Con_printf("Failed to compile map\n ");
    }
    

    engine->game_table.game_init();
    while(!gSys_window->should_close){
        
        IN_Update();
    if (in_keydown[SDL_SCANCODE_W])
    {
      VectorMA(r_pos, 0.001f, r_vfront, r_pos );
    }
        R_Start( cl.viewangles );
    
        R_DrawModel( test_mdl, MATRIX_IDENTITY);
            
        R_Present();

        engine->game_table.game_update( NULL, 2.0f );
    }
    engine->game_table.game_shutdown();

    R_Destroy();
    Sys_Exit();
    return 0;
}
