#ifndef MAP_H
#define MAP_H

#include "common/common.h"

typedef struct plane_t plane_t;
typedef struct brushface_t brushface_t;
typedef struct bspnode_t bspnode_t;

typedef struct mapdata_t
{
    bspnode_t* nodes;
    int numnodes;

    bspnode_t* leaves;
    int numleaves;

    plane_t* planes;
    int numplanes;

    brushface_t* faces;
    int numfaces;

    char name[64];
} mapdata_t;

extern mapdata_t gCurrentMap;

// Map name as it exists in the map directory, e.g 'test.bsp'
bool MAP_Load( const char* name );
extern sysarena_t map_arena;

#endif