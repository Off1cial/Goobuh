#include "engine/system/libloader.h"
#include "engine/system/message.h"

#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>

#if defined( WINDOWS )
#define SYS_LIBRARY_EXT ".dll"
#elif defined( __APPLE__ )
#define SYS_LIBRARY_EXT ".dylib"
#else
#define SYS_LIBRARY_EXT ".so"
#endif

bool Sys_LibraryLoad( syslibrary_t* lib, const char* name )
{
    memset( lib, 0, sizeof( *lib ) );

    // Look next to the executable, not in the current working directory.
    const char* base = SDL_GetBasePath();
    if (!base) base = "";

    char path[512];
    int len = snprintf( path, sizeof( path ), "%s%s%s", base, name, SYS_LIBRARY_EXT );
    if (len < 0 || (size_t)len >= sizeof( path ))
    {
        Sys_Printf( "Library path too long for '%s'\n", name );
        return false;
    }

    lib->handle = SDL_LoadObject( path );
    if (!lib->handle)
    {
        Sys_Printf( "Failed to load '%s': %s\n", path, SDL_GetError() );
        return false;
    }
    return true;
}

sysfunc_t Sys_LibraryFunction( const syslibrary_t* lib, const char* name )
{
    if (!lib->handle) return NULL;

    sysfunc_t fn = (sysfunc_t)SDL_LoadFunction( (SDL_SharedObject*)lib->handle, name );
    if (!fn)
        Sys_Printf( "Library has no function '%s': %s\n", name, SDL_GetError() );
    return fn;
}

void Sys_LibraryUnload( syslibrary_t* lib )
{
    if (lib->handle) SDL_UnloadObject( (SDL_SharedObject*)lib->handle );
    lib->handle = NULL;
}