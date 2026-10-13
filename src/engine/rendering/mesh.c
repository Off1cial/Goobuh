#include "common/common.h"
#include "engine/rendering/renderer.h"

#include <glad/glad.h>
#include "engine/rendering/gl.h"

mesh_t* RM_Alloc( unsigned int numvertices )
{
    unsigned int indices = ( numvertices - 2) * 3; // Something like that?
    
    mesh_t* m = Hunk_Alloc( sizeof(mesh_t) );
    m->verts = Hunk_Alloc( sizeof(vertex_t) * numvertices );
    m->indices = Hunk_Alloc( sizeof(unsigned int ) * indices );

    m->numvertices = 0;
    m->maxvertices = numvertices;

    m->maxindices = indices;
    m->numindices = 0;


    return m;

}


mesh_t* RM_NewMesh( unsigned int numvertices )
{
    mesh_t* m = RM_Alloc( numvertices );

    GL_CreateVAO( &m->vao );
    glGenBuffers( 1, &m->vbo );
    glGenBuffers( 1, &m->ebo );
    return m;
}




void RM_Reset( mesh_t *mesh )
{
    mesh->numvertices = 0;
    mesh->numindices = 0;
}
