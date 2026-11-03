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

    
    bool compiled = MAP_Compile( "maps/new.map", "maps/first.bsp" );
    if (!compiled)
    {
        Con_printf("Failed to compile map\n ");
    }
    
    mapdata_t map_data;
    if (!MAP_Load( "first.bsp", &map_data )) Sys_Exit();
    
    R_LoadLeafs();
   
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE );
    engine->game_table.game_init();
    while(!gSys_window->should_close){
        
        CL_Frame();
        SV_Frame(); 

        engine->game_table.game_update( NULL, 2.0f );
    }
    engine->game_table.game_shutdown();

    R_Destroy();
    Sys_Exit();
    return 0;
}
