#include "engine/rendering/gl.h"
#include "common/common.h"

#include <string.h>

#include <glad/glad.h>

void GL_CreateBuffer( 
        glbuff_t *buff, 
        glbufftype_t type, 
        GLsizeiptr size, 
        const void *data, 
        glusage_t usage 
)
{
    #ifdef PARANOID
    Sys_Assert ( buff, "GL_CreateBuffer: Attempted to create buffer from null data\n" ); 
    #endif    


    memset(buff, 0, sizeof(glbuff_t));

    buff->type = type;
    buff->size = size;
    buff->usage = usage;
    
    glGenBuffers( 1, &buff->handle );
    glBindBuffer( type, buff->handle );
    glBufferData( type, size, data, usage );
    glBindBuffer( type, 0 );
}

int GL_UpdateBuffer( 
        glbuff_t *buff, 
        GLintptr offset, 
        GLsizeiptr size, 
        const void *data )
{
   if (size + offset >= buff->size) return 0; 

   
    glBindBuffer( buff->type, buff->handle );
    glBufferSubData( buff->type, offset, size, data );

   return 1;
}


void GL_BindBuffer( const glbuff_t *buff )
{
#ifdef PARANOID
    Sys_Assert( buff, "GL_BindBuffer: Attempted to bind invalid buffer\n");
#endif

    glBindBuffer( buff->usage, buff->handle );
}


void GL_BindUniformBuffer( const glbuff_t *buff, GLuint binding )
{
#ifdef PARANOID
    Sys_Assert ( buff, "GL_BindUniformBuffer: Attempted to bind null buffer\n" );
#endif
    glBindBufferBase( buff->usage, binding, buff->handle );
}

void GL_DestroyBuffer( glbuff_t *buff )
{
    glDeleteBuffers( 1, &buff->handle );
    buff->handle = 0;
}



void GL_CreateVAO( GLuint *vao )
{
    glGenVertexArrays( 1, vao );
}

void GL_BindVAO( GLuint vao )
{
    glBindVertexArray( vao );
}


void GL_DestroyVAO( GLuint *vao )
{
    glDeleteVertexArrays(1, vao );
    *vao = 0;
}
