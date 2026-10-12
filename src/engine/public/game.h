#pragma once

#include <stdint.h>

#define GAME_API_VERSION 1

#if defined( _WIN32 )
#define GAME_EXPORT __declspec( dllexport )
#else
#define GAME_EXPORT __attribute__( ( visibility( "default" ) ) )
#endif


// What does an engine mean to the game?
typedef struct engine_api_t
{
    uint32_t version, size;

    // Functions provided by the engine
    void (*cprintf) (char* fmt, ... );


    void (*sound)( int index, float volume );
    void (*sound_positional)( int index, float volume, float pos[3] );


    int (*sound_index)( char* file );
    int (*model_index)( char* file );

} engine_api_t;

// What does a game mean to the engine?
typedef struct game_api_t
{
    uint32_t version, size;

    void (*game_init)( void );
    void (*game_shutdown)( void );

    void (*game_update)( void* state, float dt );

    void (*client_think)(void);


} game_api_t;

#define GAME_API_ENTRY_NAME "GetGameAPI"

// Returns NULL if the versions don't match.
typedef void (*GetGameAPI_fn)( const engine_api_t* engine, game_api_t* out );
