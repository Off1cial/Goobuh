#include "engine/system/message.h"
#include "engine/system/allocator.h"
#include "engine/system/window.h"
#include "engine/shared/cvar.h"

#include "common/common.h"
#include <stdio.h>



[[noreturn]] void Sys_Exit( void )
{
    Cvar_Cleanup();
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


#if defined(_WIN32)
static unsigned long time_micro( void )
{
  static LARGE_INTEGER frequency;
  LARGE_INTEGER counter;

  if (frequency.QuadPart == 0)
    QueryPerformanceFrequency(&frequency);

  QueryPerformanceCounter(&counter);

  return (uint64_t)((counter.QuadPart * 1000000ULL) /
                    frequency.QuadPart);
}
#elif defined(__linux__)
#include <time.h>
static unsigned long time_micro( void )
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (uint64_t)ts.tv_sec * 1000000ULL +
         (uint64_t)ts.tv_nsec / 1000ULL;
}


#endif

unsigned long Sys_TimeMillis( void )
{
    return time_micro() / 1000.0f;
}
