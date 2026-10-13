#include <stdio.h>
#include <SDL3/SDL.h>

#include "engine/system/window.h"
#include "engine/system/allocator.h"

// Temporary
#include "rendering/renderer.h"

#include "engine.h"

#include "engine/net/net.h"
// test
#include "engine/client/cl_input.h"

sysarena_t LifeArena;
int main( void )
{



    Sys_ArenaInit( &LifeArena, HUNK_SIZE, "EngineHunk", MEMTAG_LIFE );

    engine_t* engine = Sys_ArenaAlloc( &LifeArena, sizeof(engine_t));
    Engine_LoadGameLib( engine );

    Sys_WindowCreate( &LifeArena );

    printf("Plan and plan, write and write some more until your head bleeds...\n");

    NET_Init();

    R_Init();
    engine->game_table.game_init();
    while(1){
        
        KeyUpdate();
        R_Start();

        R_Present();

        engine->game_table.game_update( NULL, 2.0f );
    }
    engine->game_table.game_shutdown();

    R_Destroy();
    Sys_WindowDestroy();
    Sys_ArenaDestroy( &LifeArena );

    return 0;
}
