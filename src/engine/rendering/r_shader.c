#include "engine/rendering/renderer.h"
#include "common/common.h"
#include "engine/system/message.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <glad/glad.h>

// Malloc'd array, must be freed afterwards
static char *read_file( const char* path )
{
    
    if (!path) return NULL;
    
    FILE* fptr;
    long  size;
    
    char *buf;

    fptr = fopen( path, "rb" ); 
    if (!fptr){
        Con_printf("Failed to read shader file, missing source\n");
        return NULL;
    }

    // determine the file size
    fseek( fptr, 0, SEEK_END );
    size = ftell( fptr );
    rewind( fptr );

    buf = malloc( size + 1); 

    fread( buf, 1, size, fptr );
    fclose( fptr );
    buf[size] = 0;
    return buf;
}

static GLuint compile_source( GLenum type, const char* src, const char *path )
{
    GLuint id;
    GLint success;
    id = glCreateShader( type );
    glShaderSource( id, 1, &src, NULL );
    glCompileShader( id );

    // Check compilation result
    glGetShaderiv( id, GL_COMPILE_STATUS, &success );
    if (!success)
    {
        char info[1024];
        glGetShaderInfoLog( id, 1024, NULL, info ); 
        glDeleteShader( id );

        Sys_Printf("Failed to compile shader, %s: %s\n", path, info ); // do I mark this function no return?
        return 0; // Wont happen
    }
    return id;
}

bool ShaderLoad( shader_t *shader, const char *vert_path, const char *frag_path )
{   

    char vert_resolved[MAX_PATH] = ASSET_DIR;
    char frag_resolved[MAX_PATH] = ASSET_DIR;
    long assetdirsize = strlen(ASSET_DIR);
    
    strncat( vert_resolved, vert_path, MAX_PATH - assetdirsize );
    strncat( frag_resolved, frag_path, MAX_PATH - assetdirsize );

    // Returns a malloc'd buffer
    char *vert_src = read_file( vert_resolved );
    char *frag_src = read_file( frag_resolved );

    GLuint vert_sh = compile_source( GL_VERTEX_SHADER, vert_src, vert_path ); 
    GLuint frag_sh = compile_source( GL_FRAGMENT_SHADER, frag_src, frag_path );
    
    // Dispose of the buffer(s)
    free( vert_src );
    free( frag_src );

    if (!vert_sh || !frag_sh ) 
        return false;

    
    shader->program = glCreateProgram();
    glAttachShader( shader->program, vert_sh );
    glAttachShader( shader->program, frag_sh );
    glLinkProgram( shader->program );

    // Check linking status
    GLint link;
    
    glGetProgramiv( shader->program, GL_LINK_STATUS, &link );
    if (!link)
    {
        char info[1024];
        glGetProgramInfoLog( shader->program, 1024, NULL, info );
    
        glDeleteProgram( shader->program );
        shader->program = 0;

        Hunk_FreeToMark(); // Free the shader, assuming the last mark came from here
                           // Which is true if only one thread can do Hunk allocations and its the same thread used to create shaders
        Sys_Crash("Shader linking failed: %s\n", info );
        return false;
    }
    glDeleteShader( vert_sh );
    glDeleteShader( frag_sh );

    // Set uniforms?
    //
    shader->model_loc = glGetUniformLocation(shader->program, "uModel");

    // not too sure on this, am i wasting too much memory
    
    size_t vertlen = strlen( vert_path );
    size_t fraglen = strlen( frag_path );

    memcpy( shader->vpath, vert_path, vertlen );
    memcpy( shader->fpath, frag_path, fraglen );
    shader->fpath[MAX_PATH - 1] = '\0';
    shader->vpath[MAX_PATH - 1] = '\0';
    return true;
}

shader_t *R_CreateShader( const char *vertex_path, const char *fragment_path )
{
    Hunk_Mark();
    shader_t *shader = Hunk_Alloc( sizeof(shader_t) );
    
    if (!ShaderLoad( shader, vertex_path, fragment_path ))
    {
        return NULL;
    }

    return shader;
}

void R_DestroyShader( shader_t *shader )
{
    // Do I improve the allocator by adding pages so i can actually release this memory
    // Or does it not matter if the shader has a lifetime equal to that of the engine
    shader->program = 0;
    shader->vpath[0] = 0;
    shader->fpath[0] = 0; 
}

void R_UseShader( shader_t *shader )
{
    glUseProgram( shader->program );
}


void R_UploadModel( shader_t *shader, mat4 mat )
{
    glUniformMatrix4fv( shader->model_loc, 1, GL_FALSE, (float*)mat);
}


