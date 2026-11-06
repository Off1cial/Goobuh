#include "engine/map/bspfile.h"
#include "engine/map/bsp.h"
#include "engine/map/map.h"



bool BSP_Write( FILE* bsp_file )
{
    if (!bsp_file) return false;

    bspheader_t header = {0};
    header.magic = BSP_MAGIC;
    header.version = BSP_VERSION;

    header.numfaces = numfaces;
    header.numplanes = numplanes;
    header.numnodes = numnodes;
    header.numleaves = numleaves;

    fwrite( &header, sizeof(header), 1, bsp_file );

    fwrite( dnodes, sizeof(bspnode_t), numnodes, bsp_file );
    fwrite( dleafs, sizeof(bspnode_t), numleaves, bsp_file );
    fwrite( dplanes, sizeof(plane_t), numplanes, bsp_file );
    fwrite( dfaces, sizeof(brushface_t), numfaces, bsp_file );
    
    printf("BSPWrite: Face0 has texture: %s\n", dfaces[0].texname);

    return !ferror(bsp_file);
}

static inline bool check_bsp_limits( bspheader_t header )
{
    return  ( header.numnodes >= MAX_BSP_NODES ) || 
            ( header.numleaves >= MAX_BSP_NODES ) ||
            ( header.numfaces >= MAX_BRUSH_FACES * MAX_BRUSHES ) ||
            ( header.numplanes >= MAX_BRUSH_FACES * MAX_BRUSHES );
}

bool BSP_Read( FILE* f, mapdata_t* out )
{
    if (!f || !out) return false;

    bspheader_t h;
    if (fread( &h, sizeof( h ), 1, f ) != 1) return false;

    if (h.magic != BSP_MAGIC || h.version != BSP_VERSION)
    {
        Con_printf( "Bad BSP file: magic %x, version %d (expected %x, %d)\n",
                    h.magic, h.version, BSP_MAGIC, BSP_VERSION );
        return false;
    }
    if (check_bsp_limits( h )) return false;

    if (fread( dnodes, sizeof( bspnode_t ), h.numnodes, f ) != (size_t)h.numnodes)  return false;
    if (fread( dleafs, sizeof( bspnode_t ), h.numleaves, f ) != (size_t)h.numleaves) return false;
    if (fread( dplanes, sizeof( plane_t ), h.numplanes, f ) != (size_t)h.numplanes) return false;
    if (fread( dfaces, sizeof( brushface_t ), h.numfaces, f ) != (size_t)h.numfaces)  return false;


    // Need a way a to distinguish the server and client, the server doesnt need faces, the client does
    // Client could get nodes, leaves and planes while joining and load faces itself
    numplanes = h.numplanes;  numnodes = h.numnodes;
    numleaves = h.numleaves;  numfaces = h.numfaces;
    
    if (out->planes) memcpy( out->planes, dplanes, sizeof(plane_t) * h.numplanes);
    else out->numplanes = 0;


    if (out->nodes)  memcpy( out->nodes, dnodes, sizeof(bspnode_t) * h.numnodes );
    else out->numnodes = 0;
    if (out->leaves) memcpy( out->leaves, dleafs, sizeof(bspnode_t) * h.numleaves );
    else out->numleaves = 0;
    if (out->faces)  memcpy( out->faces, dfaces, sizeof(brushface_t) * h.numfaces );
    else out->numfaces = 0;



    Con_printf( "Read BSP file with %d nodes, %d leaves\n", numnodes, numleaves );
    return true;
}
