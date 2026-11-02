#ifndef BSPFILE_H
#define BSPFILE_H

#define BSP_MAGIC 0x02510 
#define BSP_VERSION 1

#include <stdio.h>
#include <stdbool.h>

typedef struct bspheader_t
{
    int magic, version;
    int numnodes, numleaves, numplanes, numfaces;
} bspheader_t;


typedef struct mapdata_t mapdata_t;

bool BSP_Write( FILE* bsp_file );

bool BSP_Read( FILE* bsp_file, mapdata_t* out );


#endif
