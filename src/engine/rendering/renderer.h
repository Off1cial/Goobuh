#pragma once

#include "common/common.h"
#include "math/matrix.h"
#include "math/vector.h"

typedef struct vertex_t
{
    float   pos[3];
    float   normal[3];
    byte    col[4];
}  vertex_t;

typedef struct shader_t
{
} shader_t; 

typedef struct mesh_t
{
    unsigned int numvertices, maxvertices;
    unsigned int numindices,  maxindices;
    unsigned int vao, vbo, ebo;

    vertex_t*       verts;
    unsigned int*   indices;

} mesh_t;

#define R_VIEW 0
#define R_PROJ 1

extern mat4 r_mats[2];

extern vec3_t r_pos;
extern vec3_t r_vfront;
extern vec3_t r_vright;

extern float r_width;
extern float r_height;

// Potentially cvars in the future
extern float r_fov;
extern float r_farz;
// r_nearz will always be 0.1f

void R_Init( void );
void R_Destroy( void );
void R_Clear( void );

void R_UpdateMatrices( vec3_t view_angles );

void R_Start( void );
void R_Present( void );


