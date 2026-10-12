#include "engine.h"

#include "system/libloader.h"
#include "system/message.h"
#include "public/game.h"
#include "common/common.h"



bool Engine_LoadGameLib( engine_t* engine )
{
    if (!Sys_LibraryLoad( &engine->lib_game, "game" ))
    {
        Sys_Crash("Failed to load game library");
        return false;
    }



    engine_api_t engine_api;
    engine_api.version = GAME_API_VERSION;

    GetGameAPI_fn get_fn = (GetGameAPI_fn)Sys_LibraryFunction( &engine->lib_game, GAME_API_ENTRY_NAME );
    if (!get_fn) 
    {

        Sys_Crash("Failed to find game api signature");
        return false;
    }

    get_fn( &engine_api, &engine->game_table );

    Sys_Printf("Successfully loaded game library\n");
    return true;
}