#ifndef BSP_H
#define BSP_H

#define MAX_BRUSH_FACES 64
#define MAX_BRUSHES 4096

#define MAX_BSP_NODES (1u << 16)

#include <stdio.h>
#include "math/plane.h"
#include "engine/poly.h"

typedef struct brushface_t
{  
    int planenum;
    char texname[ 64 ];
    winding_t win;
} brushface_t;

typedef struct brush_t
{   
    vec3_t mins, maxs;
    int numfaces;
    int maxfaces;
    struct brush_t* next;
    struct brush_t* prev;
    brushface_t *faces;
} brush_t;

brush_t *AllocBrush( int numfaces );
void FreeBrush( brush_t *b );
void FreeAllBrushes( void );

void BrushCopy( brush_t* dst, brush_t *src);
void BuildBrush( brush_t *brush );

bool BrushAddFace( brush_t *b, brushface_t face );

#define CONTENTS_EMPTY -1
#define CONTENTS_SOLID -2

typedef struct bspnode_t
{
    vec3_t mins, maxs;

    int children[ 2 ];

    int numfaces;
    int firstface;
    int planenum;

} bspnode_t;


extern bspnode_t dnodes[MAX_BSP_NODES];
extern int numnodes;


extern plane_t dplanes[MAX_BRUSHES * MAX_BRUSH_FACES];
extern int numplanes;

int FindPlane( plane_t p );

extern brush_t* dbrushes;

extern brushface_t dfaces[ MAX_BRUSH_FACES * MAX_BRUSHES ];
extern int numfaces;

extern sysarena_t compile_arena;



void BSP_Run( FILE *bsp_file );


void CSG_SplitBrush( brush_t* in, plane_t plane, brush_t* out_back, brush_t* out_front );
void CSG_ClipBrushes( brush_t* a, brush_t* b ); // Clips B with A so that A does not contain any of B

#endif
