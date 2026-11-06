#include "engine/rendering/renderer.h"
#include "engine/rendering/gl.h"
#include "engine/system/window.h"
#include "engine/rendering/model.h"

#include "common/common.h"

#include "math/matrix.h"
#include "math/vector.h"
#include <math.h>

glbuff_t *gUniformbuff = NULL;

int r_numdlights;
light_gpu_t r_dlights[MAX_DLIGHTS] = {0};

mat4 r_mats[2];

vec3_t r_pos;
vec3_t r_vfront;
vec3_t r_vright;

qangle r_viewangles;

float r_width;
float r_height;

float r_fov = DEG2RAD(60.0f);
float r_farz = 4000.0f;


shader_t *r_shaderactive = NULL;

GLuint r_vao;

glbuff_t r_vbo;
GLsizeiptr r_vbo_bytes = 0;

glbuff_t r_ebo;
unsigned int r_ebo_offset = 0;

#define INTENSITY_SCALE 1000

// TEMPORARY TEST
light_t test_light = {  .type = light_point, .intensity = 900.0f,
                        .point = {.radius = 20000}
                    };



void R_Init( void )
{
    VectorSet(test_light.point.position, 800, 80, 400);
    test_light.col[0] = 230;
    test_light.col[1] = 230;
    test_light.col[2] = 200;
    test_light.col[3] = 255;






    //vec3_t up;
    //AngleBasis( r_viewangles, r_vfront, r_vright, up );
    VectorSet( r_pos, -1, 1, -2);
    Con_printf("Angles: [%0.2f, %0.2f, %0.2f]\n", r_viewangles[0], r_viewangles[1], r_viewangles[2]);
  
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glCullFace(GL_FRONT);
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
    GL_CreateBuffer( gUniformbuff, GLBUFF_UBO, UBO_SIZE, NULL, GLUSAGE_DYNAMIC );
    GL_BindUniformBuffer( gUniformbuff, 0 );

    for (int i = 0; i < MAX_DLIGHTS; i++)
    {
        r_dlights[i].type = -1;
    }


    r_shaderactive = R_CreateShader( "shaders/default.vs", "shaders/default.fs" );
    R_UseShader( r_shaderactive );

    gluniform_t test_sampler = {0};
    GL_GetUniform(r_shaderactive, "uAlbedo", GLUNIFORM_SAMPLER, &test_sampler);

    image_t* test_image = R_LoadDDS("devgrey.dds");
    test_sampler.Sampler.handle = test_image->bindless;
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
    MatrixPerspective_GL( r_fov, aspect, 1.0f, r_farz, r_mats[R_PROJ] );

    vec3_t centre;
    VectorAdd( r_pos, r_vfront, centre );
    MatrixLookAt( r_pos, centre, AXIS_Y, r_mats[R_VIEW] );
}

static inline void reset_lights( void )
{
    // Hopefully the compiler unrolls this as the length is constant?
    int i;
    for ( i = 0; i < MAX_DLIGHTS; i++)
    {
        r_dlights[i].type = -1;
    }
    r_numdlights = 0;
}

void R_FlushLights( void )
{
    GL_UpdateBuffer( gUniformbuff, UBO_OFF_COUNT, sizeof(int), &r_numdlights );

    if ( r_numdlights > 0 )
        GL_UpdateBuffer( gUniformbuff, UBO_OFF_LIGHTS,
                         sizeof(light_gpu_t) * r_numdlights, r_dlights );
}

void R_Start( vec3_t viewangles )
{
    R_UseShader(r_shaderactive);
    reset_lights();
    R_SubmitLight (&test_light);
    R_UpdateMatrices( viewangles );
    GL_UpdateBuffer( gUniformbuff, 0, sizeof(float) * 32, r_mats ); 
    R_FlushLights();
    //glUniform3fv( r_shaderactive->set.uniforms[0].location, 1, r_pos);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}





void R_Present( void )
{
    SDL_GL_SwapWindow( gSys_window->window );
}



void R_DrawModel( model_t *mdl, mat4 model )
{
    glUniformMatrix4fv( r_shaderactive->model_loc, 1, GL_FALSE, (float*)model);
    glBindVertexArray( r_vao );

    glDrawElementsBaseVertex(GL_TRIANGLES, mdl->numindices, GL_UNSIGNED_INT,
                         (void*)((size_t)mdl->index_offset * sizeof(unsigned int)),
                         (GLint)mdl->vertex_offset);
}


void R_DrawNode( int node )
{

}

static void light_to_gpu(const light_t *src, light_gpu_t *dst)
{
    memset(dst, 0, sizeof(*dst));
    dst->type = src->type;
    memcpy(dst->col, src->col, 4);
    dst->intensity = (src->type != light_env) ? src->intensity * INTENSITY_SCALE : src->intensity;

    switch (src->type) {
    case light_spot:
        VectorCopy( src->spot.position, dst->position );
        VectorCopy( src->spot.direction, dst->direction );
        dst->cutoff =  cosf (src->spot.cutoff);
        break;
    case light_env:
        VectorCopy( src->env.direction, dst->direction );
        break;
    case light_point:
        VectorCopy( src->point.position, dst->position );
        dst->radius = src->point.radius;
        break;
    }
}



bool R_SubmitLight( light_t *light )
{
    if ( r_numdlights >= MAX_DLIGHTS )
        return false;

    light_to_gpu( light, &r_dlights[r_numdlights++] );
    return true;
}
