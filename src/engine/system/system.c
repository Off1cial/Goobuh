#include "engine/system/message.h"
#include "engine/system/allocator.h"
#include "engine/system/window.h"

#include "common/common.h"
#include <stdio.h>



void Sys_Exit( void )
{
    Sys_WindowDestroy();
    Sys_ArenaShutdownAll();

    Con_printf("Goodbye!");
    exit(0);
}

void Sys_Assert( bool condition, char *fmt, ... )
{
    if (!condition){
        va_list args;
        va_start( args, fmt );
        vprintf( fmt, args );
        va_end( args );
    }
}
