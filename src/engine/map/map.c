#include "engine/map/map.h"
#include "engine/map/bsp.h"
#include "engine/map/bspfile.h"
#include "common/common.h"
#include <stdlib.h>

sysarena_t map_arena;

size_t maphunk_size = ( 2 * MAX_BSP_NODES * sizeof(bspnode_t) ) + 
                        ( sizeof(plane_t) * MAX_BRUSHES * MAX_BRUSH_FACES ) +
                        ( sizeof(brushface_t) * MAX_BRUSH_FACES * MAX_BRUSHES );


bool MAP_Load( const char* name, mapdata_t* out )
{
    if (!name || !out) return false;

    char mapdir[] = ASSET_DIR "maps/";

    char wholedir[MAX_QPATH];
    snprintf(wholedir, MAX_QPATH, "%s%s", mapdir, name );

    if (!map_arena.base)
    {
        Sys_ArenaInit( &map_arena, maphunk_size, "MapArena", MEMTAG_LEVEL );
    }
    Sys_ArenaReset( &map_arena );

    FILE* bsp_file = fopen( wholedir, "rb" );
    if (!bsp_file)
    {
        Con_printf("Failed to load map file, \'%s\', file not found.\n", name);
        return false;
    }

    mapdata_t read = {0};

    bool ok = BSP_Read( bsp_file, &read );
    fclose(bsp_file);
    if (!ok) return false;

    Con_printf( "Read BSP file with %d nodes, %d leaves\n", numnodes, numleaves );
    *out = read;
    return true;
}

