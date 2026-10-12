#pragma once

#include <stdbool.h>

// A loaded shared library (DLL / .so). The caller owns the struct.
typedef struct syslibrary_t
{
    void* handle;   // NULL when not loaded
} syslibrary_t;

// Generic function pointer type. Cast it to the real signature before calling.
typedef void (*sysfunc_t)(void);

// Loads <folder of the exe>/<name>.dll (or .so / .dylib). Pass "game", not "game.dll".
bool      Sys_LibraryLoad( syslibrary_t* lib, const char* name );

// Returns NULL if the library has no exported function with that name.
sysfunc_t Sys_LibraryFunction( const syslibrary_t* lib, const char* name );

// After this, every pointer obtained from the library is dangling.
void      Sys_LibraryUnload( syslibrary_t* lib );