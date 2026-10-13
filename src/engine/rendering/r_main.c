#include "engine/rendering/renderer.h"
#include "engine/rendering/gl.h"
#include "engine/system/window.h"

#include "common/common.h"

#include "math/matrix.h"
#include "math/vector.h"
#include <math.h>

glbuff_t *gUniformbuff = NULL;


mat4 r_mats[2];

vec3_t r_pos;
vec3_t r_vfront;
vec3_t r_vright;

float r_width;
float r_height;

float r_fov = 60.0f;
float r_farz = 1000.0f;

void R_Init( void )
{
    VectorSet( r_pos, 0, 0, 0);
    VectorSet( r_vfront, 0, 0, -1 ); // -Z
    VectorSet( r_vright, -1, 0, 0 ); // I think -X is correct

    MatrixIdentity( r_mats[R_VIEW] );
    MatrixIdentity( r_mats[R_PROJ] );
    
    r_width = gSys_window->width;
    r_height = gSys_window->height;

    gUniformbuff = Hunk_Alloc( sizeof(glbuff_t) );
    GL_CreateBuffer( gUniformbuff, GLBUFF_UBO, UBO_SIZE, r_mats, GLUSAGE_DYNAMIC );
    GL_BindUniformBuffer( gUniformbuff, 0 );
}

void R_Destroy( void )
{
}

void R_UpdateMatrices( vec3_t view_angles )
{
    float   aspect;
    vec3_t  up;

    AngleBasis( view_angles, r_vfront, r_vright, up );
    VectorNormalise(r_vfront);
    VectorNormalise(r_vright);

    aspect = r_width / (float)r_height;
    MatrixPerspective_GL( r_fov, aspect, 0.1f, r_farz, r_mats[R_VIEW] );

    vec3_t centre;
    VectorAdd(r_pos, r_vfront, centre );
    MatrixLookAt( r_pos, centre, AXIS_Y, r_mats[R_VIEW] );
}

void R_Start( void )
{
    GL_UpdateBuffer( gUniformbuff, 0, UBO_SIZE, r_mats ); 
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}


void R_Present( void )
{

    SDL_GL_SwapWindow( gSys_window->window );
}
