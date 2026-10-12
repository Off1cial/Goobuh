#include "engine/system/message.h"
#include "engine/system/allocator.h"
#include "engine/system/window.h"

#include "common/common.h"



void Sys_Exit( void )
{
    Sys_WindowDestroy();
    Sys_ArenaShutdownAll();

    Con_printf("Goodbye!");
    exit(0);
}
