#ifndef RENDERER_GL_H
#define RENDERER_GL_H


#include <glad/glad.h>
#include "common/common.h"
// Various OpenGL wrappers

#include "engine/rendering/r_light.h"

typedef struct image_t image_t;


typedef enum 
{
    GLBUFF_VBO = GL_ARRAY_BUFFER,
    GLBUFF_EBO = GL_ELEMENT_ARRAY_BUFFER,
    GLBUFF_UBO = GL_UNIFORM_BUFFER,
} glbufftype_t;

typedef enum
{
    GLUSAGE_STATIC = GL_STATIC_DRAW,
    GLUSAGE_DYNAMIC = GL_DYNAMIC_DRAW,
} glusage_t;

typedef struct 
{
    GLuint       handle;
    glbufftype_t type;
    GLsizeiptr   size;
    glusage_t    usage;
    GLuint       binding;
} glbuff_t;


typedef struct ubo_block_t
{
    float       view[16];                  // 0
    float       projection[16];            // 64
    int32_t     light_count;               // 128
    int32_t     _pad[3];                   // 132..143
    light_gpu_t lights[MAX_DLIGHTS];       // 144
} ubo_block_t;

_Static_assert(OFFSETOF(ubo_block_t, light_count) == 128, "light_count");
_Static_assert(OFFSETOF(ubo_block_t, lights) == 144, "lights");

#define UBO_OFF_MATS    0                    // view + projection (128 bytes)
#define UBO_OFF_COUNT   128                  // int lightCount
#define UBO_OFF_LIGHTS  144                  // light_gpu_t[MAX_DLIGHTS]
#define UBO_MIN_SIZE    (UBO_OFF_LIGHTS + sizeof(light_gpu_t) * MAX_DLIGHTS)

_Static_assert(UBO_MIN_SIZE >= UBO_MIN_SIZE, "UBO_SIZE too small");

// Keep updated!
#define UBO_SIZE UBO_MIN_SIZE
#define STATIC_VBO_SIZE (16 * 1024 * 1024)
#define STATIC_EBO_SIZE (16 * 1024 * 1024)
extern glbuff_t *gUniformbuff;

extern GLuint r_vao;
extern glbuff_t r_vbo;
extern glbuff_t r_ebo;
extern GLsizeiptr r_vbo_bytes;
extern unsigned int r_ebo_offset;


#define MAX_GL_UNIFORMS 16

typedef enum gluniformtype_t
{
    GLUNIFORM_MAT4,
    GLUNIFORM_VEC3,
    GLUNIFORM_ARRAY,
} gluniformtype_t;


typedef struct
{
    char name[32];
    int location;
    short type;
    
    union
    {
        struct 
        {
            float xyz[3];
        } Vector3;

        struct 
        {
            float m[16];
        } Mat4;

        struct 
        {
            int len;
            byte data[];
        } Array;
    };

} gluniform_t;


typedef struct
{
    gluniform_t uniforms[MAX_GL_UNIFORMS];
    int numuniforms;
} gluniformset_t;

typedef struct shader_t
{
    gluniformset_t set;
    unsigned int program;

    int model_loc;

    char vpath[MAX_QPATH];
    char fpath[MAX_QPATH];

} shader_t; ;

bool GL_AddUniform( gluniformset_t* set, gluniform_t uni );
bool GL_GetUniform( shader_t* shader, const char* name32, gluniformtype_t utype, gluniform_t* out );




void GL_SetTexture( image_t* image );
void GL_BindTexture( image_t* image ); // Use bindless textures?


void GL_CreateBuffer( glbuff_t *buff, glbufftype_t type, GLsizeiptr size, const void *data, glusage_t usage );

void GL_BindBuffer( const glbuff_t *buff );

void GL_DestroyBuffer( glbuff_t *buff );

int GL_UpdateBuffer( glbuff_t *buff, GLintptr offset, GLsizeiptr size, const void *data );


void GL_CreateVAO( GLuint *vao );
void GL_BindVAO( GLuint voa );
void GL_DestroyVAO( GLuint *vao );

void GL_BindUniformBuffer( const glbuff_t *buff, GLuint binding );

#endif
