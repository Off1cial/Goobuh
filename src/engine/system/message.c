#include "engine/system/message.h"

#include "common/common.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <SDL3/SDL.h>

void Sys_MessageBoxV( const char* fmt, va_list args )
{
    char msg[256];
    vsnprintf( msg, sizeof( msg ), fmt, args );
    SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Problem encountered", msg, NULL );
}

void Sys_MessageBox( const char* fmt, ... )
{
    va_list args;
    va_start( args, fmt );
    Sys_MessageBoxV( fmt, args );
    va_end( args );
}

[[noreturn]] void Sys_Crash( const char* fmt, ... )
{
    va_list args;
    va_start( args, fmt );
    Sys_MessageBoxV( fmt, args );
    va_end( args );

    Sys_Exit();
}

void Sys_Printf( const char* fmt, ... )
{
    va_list args;
    va_start( args, fmt );
    vprintf(fmt, args);
    va_end( args );
}

void Sys_Error( char* fmt, ... )
{
    va_list args;
    va_start( args, fmt );
    vprintf(fmt, args);
    va_end( args );
}
