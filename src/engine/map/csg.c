#include "common/common.h"
#include "engine/map/bsp.h"
#include "math/plane.h"

void CSG_SplitBrush( brush_t* in, plane_t plane, brush_t* out_back, brush_t* out_front )
{
    if (!in) return;
    if (!out_back || out_front ) return;

    out_back->numfaces = 0;
    out_front->numfaces = 0;

    int splitnum = FindPlane( plane );
    plane_t reversed_plane;
    PlaneReverse( plane, &reversed_plane );

    int bface;
    for (bface = 0; bface < in->numfaces; bface++)
    {
        winding_t front = {0}, back = {0};
        brushface_t *face = &in->faces[bface];
        ClipWinding( &face->win, plane, &back );
        ClipWinding( &face->win, reversed_plane, &front );

        if (front.numpoints > 3 )
        {
            brushface_t front_face = {0};
            front_face.planenum = face->planenum;
            front_face.win = front;
            BrushAddFace( out_front, front_face );
        }
        if (back.numpoints > 3 )
        {
            brushface_t back_face = {0};
            back_face.planenum = face->planenum;
            back_face.win = back;
            BrushAddFace( in, back_face );
        }
    }
    // Seal the cut
    brushface_t seal_front = {0}, seal_back = {0};
    seal_front.planenum = FindPlane( reversed_plane );
    seal_back.planenum = splitnum;
    BrushAddFace( out_front, seal_front );
    BrushAddFace( out_back, seal_back );
    BuildBrush( out_back );
    BuildBrush( out_front );
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


