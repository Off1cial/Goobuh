#include <stdarg.h>
#include "core/defs.h"
#include "core/assert.h"

void _assert( void* ptr, const char* fmt, ... )
{
    if (ptr != NULL)
        return;
    va_list args;
    va_start( args, fmt );
    vprintf( fmt, args );
    va_end( args );
    exit(1);
}


void _assert( int cond, const char* fmt, ... )
{
    if (cond) 
        return;
    va_list args;
    va_start( args, fmt );
    vprintf( fmt, args );
    va_end( args );
    exit(1);
}
