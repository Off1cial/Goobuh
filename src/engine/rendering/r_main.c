#include "engine/rendering/renderer.h"
#include "engine/rendering/gl.h"
#include "engine/system/window.h"
#include "engine/rendering/model.h"

#include "common/common.h"

#include "math/matrix.h"
#include "math/vector.h"
#include <math.h>

glbuff_t *gUniformbuff = NULL;


mat4 r_mats[2];

vec3_t r_pos;
vec3_t r_vfront;
vec3_t r_vright;

qangle r_viewangles;

float r_width;
float r_height;

float r_fov = DEG2RAD(60.0f);
float r_farz = 1000.0f;


shader_t *r_shaderactive = NULL;

GLuint r_vao;

glbuff_t r_vbo;
GLsizeiptr r_vbo_bytes = 0;

glbuff_t r_ebo;
unsigned int r_ebo_offset = 0;

void R_Init( void )
{
    //vec3_t up;
    //AngleBasis( r_viewangles, r_vfront, r_vright, up );
    VectorSet( r_pos, -1, 1, -2);
    Con_printf("Angles: [%0.2f, %0.2f, %0.2f]\n", r_viewangles[0], r_viewangles[1], r_viewangles[2]);
  
    glEnable(GL_DEPTH_TEST);
    MatrixIdentity( r_mats[R_VIEW] );
    MatrixIdentity( r_mats[R_PROJ] );
    
    r_width = gSys_window->width;
    r_height = gSys_window->height;
    // Create VAO
    GL_CreateVAO( &r_vao );
    // Create VBO for static geometry
    GL_CreateBuffer( &r_vbo, GLBUFF_VBO, STATIC_VBO_SIZE, NULL, GLUSAGE_STATIC );
    glBindVertexArray( r_vao );
    glBindBuffer( GL_ARRAY_BUFFER, r_vbo.handle );
    glEnableVertexAttribArray(0);
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)OFFSETOF(vertex_t, pos ));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)OFFSETOF(vertex_t, normal ));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)OFFSETOF(vertex_t, uv ));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer( 3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(vertex_t), (void*)OFFSETOF(vertex_t, col));

    // Create EBO
    GL_CreateBuffer( &r_ebo, GLBUFF_EBO, STATIC_EBO_SIZE, NULL, GLUSAGE_STATIC );
    glBindVertexArray( r_vao );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, r_ebo.handle );

    gUniformbuff = Hunk_Alloc( sizeof(glbuff_t) );
    GL_CreateBuffer( gUniformbuff, GLBUFF_UBO, UBO_SIZE, r_mats, GLUSAGE_DYNAMIC );
    GL_BindUniformBuffer( gUniformbuff, 0 );

    r_shaderactive = R_CreateShader( "shaders/default.vs\0", "shaders/default.fs\0" );
    R_UseShader( r_shaderactive );
}

void R_Destroy( void )
{
    GL_DestroyBuffer( gUniformbuff );
}

void R_UpdateMatrices( vec3_t view_angles )
{
    float   aspect;
    vec3_t  up;

    AngleBasis( view_angles, r_vfront, r_vright, up );
    VectorNormalise(r_vfront);
    VectorNormalise(r_vright);

    aspect = r_width / (float)r_height;
    MatrixPerspective_GL( r_fov, aspect, 0.1f, r_farz, r_mats[R_PROJ] );

    vec3_t centre;
    VectorAdd( r_pos, r_vfront, centre );
    MatrixLookAt( r_pos, centre, AXIS_Y, r_mats[R_VIEW] );
}

void R_Start( vec3_t viewangles )
{
    R_UpdateMatrices( viewangles );
    GL_UpdateBuffer( gUniformbuff, 0, UBO_SIZE, r_mats ); 
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}


void R_Present( void )
{

    SDL_GL_SwapWindow( gSys_window->window );
}



void R_DrawModel( model_t *mdl, mat4 model )
{
    R_UploadModel( r_shaderactive, model );
    glBindVertexArray( r_vao );

    glDrawElementsBaseVertex(GL_TRIANGLES, mdl->numindices, GL_UNSIGNED_INT,
                         (void*)((size_t)mdl->index_offset * sizeof(unsigned int)),
                         (GLint)mdl->vertex_offset);
}


