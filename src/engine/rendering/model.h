#ifndef MODEL_H
#define MODEL_H

#include "common/common.h"
typedef struct vertex_t vertex_t;

// Temporary data used when loading static geometry from files
typedef struct model_load_t
{
    int numvertices;
    int maxvertices;

    int numindices;
    int maxindices;

    vertex_t* vertices;
    unsigned int* indices; // local, add the index offset to access from global ebo
    char name[64];
} model_load_t;

typedef struct model_t
{
    qhandle_t key; // [ bucket index | index in bucket ]
    unsigned int numindices;
    unsigned int index_offset; // first index
    unsigned int vertex_offset; // first vertex

    unsigned int* indices;
    struct model_t *next;
    char name[ 64 ];
} model_t;

#define MDL_BUCKETS 64

extern model_t gModels[MDL_BUCKETS];

model_t* MDL_Load( char* path );

#endif
