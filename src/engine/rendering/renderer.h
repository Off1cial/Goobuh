#pragma once

#include "common/common.h"
#include "math/matrix.h"
#include "math/vector.h"

typedef struct vertex_t
{
    float   pos[3];
    float   normal[3];
    float   uv[2];
    byte    col[4];
}  vertex_t;

typedef struct shader_t
{
    unsigned int program;
    
    int model_loc;

    char vpath[MAX_PATH];
    char fpath[MAX_PATH];

} shader_t; 

typedef struct model_t model_t;

// r_shader.c
shader_t *R_CreateShader( const char *vertex_path, const char *fragment_path ); 
void R_DestroyShader( shader_t *shader );
void R_UseShader( shader_t *shader );

void R_UploadModel( shader_t *shader, mat4 mat );




#define R_VIEW 0
#define R_PROJ 1

extern mat4 r_mats[2];

extern vec3_t r_pos;
extern vec3_t r_vfront;
extern vec3_t r_vright;

extern qangle r_viewangles;

extern float r_width;
extern float r_height;

// Potentially cvars in the future
extern float r_fov;
extern float r_farz;
// r_nearz will always be 0.1f
//

extern shader_t *r_shaderactive;

void R_Init( void );
void R_Destroy( void );
void R_Clear( void );

void R_UpdateMatrices( vec3_t view_angles );

void R_Start( vec3_t viewangles );
void R_Present( void );

void R_DrawModel( model_t *mdl, mat4 model );


