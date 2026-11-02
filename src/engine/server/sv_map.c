#include "engine/server/server.h"
#include "engine/map/map.h"
#include "engine/map/bsp.h"

/* Preprare the server to load maps */
bool SV_InitMaps( void )
{
    sv.mapdata = SV_Alloc( sizeof(mapdata_t) );
    if (!sv.mapdata) return false;

    sv.mapdata->nodes = SV_Alloc( sizeof(bspnode_t) * MAX_BSP_NODES );
    sv.mapdata->leaves = SV_Alloc( sizeof(bspnode_t) * MAX_BSP_NODES );
    sv.mapdata->planes = SV_Alloc( sizeof(plane_t) * MAX_BRUSHES * MAX_BRUSH_FACES );

    if (!sv.mapdata->nodes || !sv.mapdata->leaves || !sv.mapdata->planes ) return false;
    return true;
}

bool SV_ChangeMap( const char* map_name )
{
    return MAP_Load( map_name, sv.mapdata ); 
}
