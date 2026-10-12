#ifndef ENGINE_H
#define ENGINE_H

#include "system/libloader.h"
#include "public/game.h"

typedef struct engine_t
{
    syslibrary_t lib_game;
    game_api_t game_table;

} engine_t;



bool EN_Init( void );


bool Engine_LoadGameLib( engine_t* engine );

#endif