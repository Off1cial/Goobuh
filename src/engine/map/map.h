#ifndef MAP_H
#define MAP_H

typedef struct bspnode_t bspnode_t;

typedef struct mapdata_t
{
    bspnode_t* nodes;
    int numnodes;

    char name[64];
} mapdata_t;

extern mapdata_t gCurrentMap;

// bspfile.c
bool MAP_LoadBSP( const char *path );



#endif