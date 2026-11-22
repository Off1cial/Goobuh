#include "core/error.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
// Essentially a simple printf until a fancy log system is in place


void Error (const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}


