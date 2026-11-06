#include "common/common.h"
#include "engine/rendering/renderer.h"
#include "engine/map/bsp.h"
#include "engine/rendering/gl.h"

static leafdraw_t ldraws[MAX_BSP_NODES];

static void R_CalculateUV(
    const brushface_t *face,
    const vec3_t pos,
    int texture_width,
    int texture_height,
    float uv[2] );

void R_LoadLeafs( void )
{
    for (int i  = 0; i < numleaves; i++)
    {
        bspnode_t* leaf = &dleafs[i];
        ldraws[i].first_index = r_ebo_offset;
        // Add face windings

        for (int f = 0; f < leaf->numfaces; f++)
        {
            const brushface_t* face = &dfaces[leaf->firstface + f ];
            const winding_t* win = &face->win;
            const plane_t plane = dplanes[face->planenum];
            const int numverts = win->numpoints;
            const unsigned base = r_vbo_bytes / sizeof(vertex_t);
            vertex_t verts[MAX_WINDING_POINTS] = {0};

            
            image_t* image = R_FindImage(face->texname);
            if (!image)
            {
                printf("Failed to find image with name: %s\n", face->texname);
                Sys_Exit();
            }

            for (int v = 0; v < numverts; v++)
            {
             
                vertex_t *vert = &verts[v];
                VectorCopy( win->p[v], vert->pos );
                VectorCopy( plane.normal, vert->normal );
                vert->col[0] = 255;
                vert->col[1] = 255;
                vert->col[2] = 255;
                vert->col[3] = 255;
                // Normals, colours, uvs?
                if(image)R_CalculateUV(
                        face, win->p[v], image->width, image->height, vert->uv
                        );
            }

            GL_UpdateBuffer( &r_vbo, r_vbo_bytes, sizeof(vertex_t) * numverts, verts );
            r_vbo_bytes += sizeof(vertex_t) * numverts;

            for (int v = 1; v + 1 < numverts; v++)
            {
                unsigned tri[3] = { base, base + v, base + v + 1};
                GL_UpdateBuffer( &r_ebo, r_ebo_offset * sizeof(unsigned), sizeof(unsigned) * 3, tri );
                r_ebo_offset += 3;
            }
        }
        ldraws[i].num_indices = r_ebo_offset - ldraws[i].first_index;
    }
}


void R_DrawLeaf( int leafnum )
{
    R_UploadModel( r_shaderactive, MATRIX_IDENTITY );
    leafdraw_t* d = &ldraws[leafnum];
    if (!d->num_indices) return;
    glBindVertexArray( r_vao );
    glDrawElements( GL_TRIANGLES, d->num_indices, GL_UNSIGNED_INT,
                    (void*)((size_t)d->first_index * sizeof( unsigned )) );
}



void R_DrawLeaves( void )
{
    R_UploadModel( r_shaderactive, MATRIX_IDENTITY );
    glBindVertexArray( r_vao );
    for (int l = 0; l < numleaves; l++)
    {
        leafdraw_t *draw = &ldraws[l];
        if (!draw->num_indices) continue;
        glDrawElements( GL_TRIANGLES, draw->num_indices, GL_UNSIGNED_INT, (void*)((size_t)draw->first_index * sizeof(unsigned)) );
    }
}


static void R_CalculateUV(
    const brushface_t *face,
    const vec3_t pos,
    int texture_width,
    int texture_height,
    float uv[2] )
{
    uv[0] = (
        DotProduct(pos, face->tex_s) / face->tex_scale[0]
        + face->tex_offset[0]
    ) / texture_width;

    uv[1] = (
        DotProduct(pos, face->tex_t) / face->tex_scale[1]
        + face->tex_offset[1]
    ) / texture_height;
}
