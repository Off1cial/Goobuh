#include "common/common.h"
#include "engine/map/bsp.h"
#include "math/plane.h"

void CSG_SplitBrush( brush_t* in, plane_t plane, brush_t* out_back, brush_t* out_front )
{
    if (!in) return;

    if (!out_front || !out_back) return;

    plane_t reverse_plane;
    PlaneReverse( plane, &reverse_plane );
    
    memset(out_back, 0, sizeof(*out_back));
    memset(out_front, 0, sizeof(*out_front));

    int face;

    for (face = 0; face < in->numfaces; face++)
    {
        // Unholy stack usage?
        winding_t front;
        winding_t back;
        ClipWinding( &in->faces[face].win, plane, &back );
        ClipWinding( &in->faces[face].win, reverse_plane, &front );

        if (WINDING_VALID( front ))
        {
            brushface_t front_face = {0};
            memcpy( front_face.texname, in->faces[face].texname, 64 );
            front_face.win = front;
            int planenum = plane_exists(reverse_plane); 
            if (planenum < 0)
            {
                front_face.planenum = numplanes;
                // Bounds checking??
                dplanes[numplanes++] = reverse_plane;
            }
            else
            {
                front_face.planenum = planenum;
            }  
            BrushAddFace( out_front, front_face );
        }
        if (WINDING_VALID( back ))
        {
            brushface_t back_face = {0};
            memcpy( back_face.texname, in->faces[face].texname, 64 );
            back_face.win = back;
            int planenum = plane_exists( plane );
            if (planenum < 0)
            {
                back_face.planenum = numplanes;
                dplanes[numplanes++] = plane;
            }
            else
            {
                back_face.planenum = planenum;
            }
            BrushAddFace( out_back, back_face );
        }
    }
}


void CSG_ClipBrushes( brush_t* a, brush_t* b )
{
    if (!a || !b) return;
    // Clip each face of b by each face of a 
    int b_face, a_face;
    for (b_face = 0; b_face < b->numfaces; b_face++)
    {   
        brushface_t* subject = &b->faces[b_face];
        winding_t cur = subject->win;
        for (a_face = 0; a_face < a->numfaces; a_face++)
        {
            plane_t clip_plane;
            PlaneReverse( dplanes[a->faces[a_face].planenum], &clip_plane );
            winding_t clipped;
            ClipWinding( &cur, clip_plane, &clipped );
            cur = clipped;
            if (cur.numpoints < 3) break;
        }
        subject->win = cur;
    }
}


