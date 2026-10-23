#ifndef RENDERER_GL_H
#define RENDERER_GL_H


#include <glad/glad.h>
#include "common/common.h"
// Various OpenGL wrappers

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

typedef struct
{
    float view[16];
    float proj[16];
} uniformlayout_t;

// Keep updated!
#define UBO_SIZE (sizeof(float) * 32)
#define STATIC_VBO_SIZE (16 * 1024 * 1024)
#define STATIC_EBO_SIZE (16 * 1024 * 1024)
extern glbuff_t *gUniformbuff;

extern GLuint r_vao;
extern glbuff_t r_vbo;
extern glbuff_t r_ebo;
extern GLsizeiptr r_vbo_bytes;
extern unsigned int r_ebo_offset;

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
