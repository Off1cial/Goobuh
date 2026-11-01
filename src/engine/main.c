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

#include "engine/server/server.h"

#include "engine/map/compile.h"
#include "engine/rendering/model.h"
#include "engine/map/bsp.h"
#include "engine/map/map.h"

sysarena_t LifeArena;
int main( void )
{

    

    Sys_ArenaInit( &LifeArena, HUNK_SIZE, "EngineHunk", MEMTAG_LIFE );

    engine_t* engine = Sys_ArenaAlloc( &LifeArena, sizeof(engine_t));
    Engine_LoadGameLib( engine );

    Sys_WindowCreate( &LifeArena );

    R_Init();
    NET_Init();
    is_server = SV_Init( 24 );
    is_client = CL_Init();
    //model_t* test_mdl = MDL_Load("models/test.mdl");

    /*
    bool compiled = MAP_Compile( "maps/new.map", "maps/first.bsp" );
    if (!compiled)
    {
        Con_printf("Failed to compile map\n ");
    }
    */
    if (!MAP_Load( "first.bsp" )) exit(1);
    
    R_LoadLeafs();
   
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE );
    engine->game_table.game_init();
    while(!gSys_window->should_close){
        


        IN_Update();
        if (in_keydown[SDL_SCANCODE_W])
        {
            VectorMA(r_pos, 0.8f, r_vfront, r_pos );
        }

        //Con_printf("In node: %d\n", FindLeaf( r_pos ));

        int current_leaf = FindLeaf( r_pos );
        printf("Leaf contents = %d\n", dleafs[current_leaf].contents );

        R_Start( cl.viewangles );
    
        //R_DrawModel( test_mdl, MATRIX_IDENTITY);
        R_DrawLeaves();

        R_Present();

        engine->game_table.game_update( NULL, 2.0f );
    }
    engine->game_table.game_shutdown();

    R_Destroy();
    Sys_Exit();
    return 0;
}
