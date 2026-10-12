#include <stdio.h>
#include <SDL3/SDL.h>

#include "engine/system/window.h"
#include "engine/system/allocator.h"

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
    engine->game_table.game_init();
    while(1){
        
        KeyUpdate();


        engine->game_table.game_update( NULL, 2.0f );
        glClearColor( 0.1f, 0.2f, 0.4f, 1.0f );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        SDL_GL_SwapWindow( gSys_window->window );
    }
    engine->game_table.game_shutdown();

    Sys_WindowDestroy();
    Sys_ArenaDestroy( &LifeArena );

    return 0;
}