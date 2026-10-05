#include "physics/backend/simd.h"
#include <string.h>
#include <stdlib.h>


// Returns the number of remaining floats to be worked on sequentially
int AVX_Add( float* dst, float* a, float* b, int count );
int AVX_IntegrateFMA3( float* dst, float* a, float* b, float m, int count );
int AVX_Integrate( float* dst, float* a, float* b, float m, int count );


simdvec3_t SIMDVEC3_New( size_t count )
{
    simdvec3_t new = { 0 };
    new.count = count;


    new.x = calloc( count, sizeof( float ) );
    new.y = calloc( count, sizeof( float ) );
    new.z = calloc( count, sizeof( float ) );

    return new;

}



void SIMDVEC3_Free( simdvec3_t* v )
{
    if (!v)
        return;

    v->count = 0;
    if (v->x) free( v->x );
    if (v->y) free( v->y );
    if (v->z) free( v->z );
}


int SIMDVEC3_Add( simdvec3_t* out, simdvec3_t* a, simdvec3_t* b, int count )
{
  // Compare CPU Features in this or prior to device which functions to use


    if (!out || !a || !b || (count <= 0))
        return 0;
    int remain = AVX_Add( out->x, a->x, b->x, count );
    AVX_Add( out->y, a->y, b->y, count );
    AVX_Add( out->z, a->z, b->z, count );
    int start = count - remain;
    // Linear finish
    for (int i = start; i < count; i++)
    {
        out->x[i] = a->x[i] + b->x[i];
        out->y[i] = a->y[i] + b->y[i];
        out->z[i] + a->z[i] + b->z[i];
    }


    return 1;
}


int SIMDVEC3_Integrate( simdvec3_t* out, simdvec3_t* a, simdvec3_t* b, float m, int count )
{
    if (!out || !a || !b || (count <= 0))
        return 0;

    int remain = AVX_IntegrateFMA3( out->x, a->x, b->x, m, count );
    AVX_IntegrateFMA3( out->y, a->y, b->y, m, count );
    AVX_IntegrateFMA3( out->z, a->z, b->z, m, count );
    int start = count - remain;
    for (int i = start; i < count; i++)
    {
        out->x[i] = a->x[i] + (m * b->x[i]);
        out->y[i] = a->y[i] + (m * b->y[i]);
        out->z[i] = a->z[i] + (m * b->z[i]);
    }
    return 1;
}