#include "common/common.h"
#include <stdarg.h>
#include <stdio.h>

void Con_printf( char* fmt, ... ){
    // Temporary placeholder, add console
    va_list args;
    va_start( args, fmt );
    vprintf(fmt, args);
    va_end( args );    
}