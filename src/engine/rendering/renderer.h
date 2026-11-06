#pragma once

#include "common/common.h"
#include "engine/system/window.h"
#include "math/matrix.h"
#include "math/vector.h"
#include "engine/rendering/r_light.h"

typedef struct vertex_t
{
    float   pos[3];
    float   normal[3];
    float   uv[2];
    byte    col[4];
}  vertex_t;


#define MAX_IMAGES 512
typedef struct image_t
{
    char name[64]; // Used for access, e.g render texture "MAP1_WALL0"
    int width, height;
    GLuint64 bindless;
    struct image_t *next;
    unsigned int glnum;
} image_t;

image_t* R_FindImage( const char* name );

image_t *R_LoadDDS( const char* filename );
void R_FreeImage (image_t *image );

typedef struct leafdraw_t
{
    int first_index;
    int num_indices;
} leafdraw_t;

typedef struct model_t model_t;

typedef struct shader_t shader_t;

// r_shader.c
shader_t *R_CreateShader( const char *vertex_path, const char *fragment_path ); 
void R_DestroyShader( shader_t *shader );
void R_UseShader( shader_t *shader );

void R_UploadModel( shader_t *shader, mat4 mat );

void R_DrawLeaf( int leafnum );
void R_DrawLeaves( void );
void R_LoadLeafs( void );


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

bool R_SubmitLight( light_t* light );
