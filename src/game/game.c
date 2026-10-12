#include "game.h"
#include <stdio.h>

void Game_Init( void )
{
  printf("Hello!\n");
}

void Game_Shutdown( void )
{
  printf("Goodbye!\n");
}

void Game_Update( void* state, float dt )
{
  //printf("Update dt = %f\n", dt );
}


static const engine_api_t* gi;
static const game_api_t game_api = {.version = GAME_API_VERSION, .game_init = Game_Init, .game_shutdown = Game_Shutdown, .game_update = Game_Update };



typedef struct game_state_t { int ticks; } game_state_t;


GAME_EXPORT void GetGameAPI( const engine_api_t* engine, game_api_t* out )
{
  gi = engine;
  *out = game_api;
}
