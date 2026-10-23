#include "engine/map/bsp.h"
#include "common/common.h"
#include "engine/system/window.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>

static float axial_bonus = 1.2f;
#define AXIAL_EPS 1e-6f
#define SPLIT_EPS 0.01f

enum
{
    SPLIT_NONE = 0,
    SPLIT_FRONT = 1, // Brush is entirely infront of the plane
    SPLIT_BACK  = 2, // Brush is entirely behind the plane
    SPLIT_STRADDLE = 4, // Brush is spliced by the plane
};

int PlaneAxis( const plane_t p )
{
    const float *n = p.normal;
    float ax = fabsf(n[0]), ay = fabsf(n[1]), az = fabsf(n[2]);

    if (ay < AXIAL_EPS && az < AXIAL_EPS && ax > AXIAL_EPS) return 0;
    if (ax < AXIAL_EPS && az < AXIAL_EPS && ay > AXIAL_EPS) return 1;
    if (ax < AXIAL_EPS && ay < AXIAL_EPS && az > AXIAL_EPS) return 2;
    return -1;
}

bool is_axial( plane_t plane )
{
    return PlaneAxis( plane ) != -1;
}

int plane_exists( plane_t  p )
{
    int i;
    for (i = 0; i < numplanes; i++)
    {
        if (PlaneEqual(p, dplanes[i])) return i;
    }
    return -1;
}

static int ClassifyBrush( const brush_t *b, plane_t plane )
{
    int flags = SPLIT_NONE;                     // NONE = lies on the plane
    for (int f = 0; f < b->numfaces; f++)
        for (int v = 0; v < b->faces[f].win.numpoints; v++)
        {
            float d = VectorDot( plane.normal, b->faces[f].win.p[v] ) - plane.d;
            if (d >  SPLIT_EPS) flags |= SPLIT_FRONT;
            if (d < -SPLIT_EPS) flags |= SPLIT_BACK;
        }
    return flags;
}

static float score_plane( plane_t plane, const brush_t *list )
{
    int front = 0, back = 0, split = 0;
    for (const brush_t *b = list; b; b = b->next)
    {
        int c = ClassifyBrush( b, plane );
        if (c == (SPLIT_FRONT | SPLIT_BACK)) split++;
        else if (c & SPLIT_FRONT)            front++;
        else                                 back++;      // behind, or touching
    }
    if (front + split == 0 || back + split == 0)
        return FLT_MAX;                                    // no progress, never pick

    float score = split * 5.0f + abs(front - back);        // splits dominate, balance breaks ties
    if (!is_axial( plane )) score *= 1.2f;                 // prefer axial planes
    return score;
}

int choose_plane( const brush_t *list )
{
    float best = FLT_MAX;
    int   best_plane = -1;
    for (const brush_t *b = list; b; b = b->next)
        for (int f = 0; f < b->numfaces; f++)
        {
            int pn = b->faces[f].planenum;
            float s = score_plane( dplanes[pn], list );
            if (s < best) { best = s; best_plane = pn; }
        }
    return best_plane;       // -1 = nothing useful left, make a leaf
}


static void BSP_Subtract( void )
{
    brush_t *big, *clip;
    for (big = dbrushes; big ; big = big->next )
    {
        for (clip = dbrushes; clip ; clip = clip->next )
        {
            if (clip == big )
                continue;

            CSG_ClipBrushes(big, clip);
        }
    }
}

void BSP_Run( FILE *bsp_file )
{
    if (!bsp_file) return; 

    BSP_Subtract();

}



brush_t* AllocBrush( int numfaces )
{
    brush_t *b = calloc(1, sizeof(brush_t));
    if (!b) return NULL;
    b->faces = calloc(1, sizeof(brushface_t) * numfaces);
    b->maxfaces = numfaces;
    b->numfaces = 0;
    if (!b->faces) return NULL;
    return b;
}

void FreeBrush( brush_t* b )
{
    if (!b) return;
    if (b->faces) free(b->faces);
    free(b);
}

void FreeAllBrushes( void )
{
    brush_t *current = dbrushes;
    while (current)
    {
        brush_t *next = current->next;
        Con_printf("Freeing brush\n");
        FreeBrush(current);
        current = next;
    }
}

void BrushCopy( brush_t* dst, brush_t *src)
{
    if (!dst || !src) return;
    memcpy(dst->faces, src->faces, sizeof(brushface_t) * src->numfaces);
    dst->numfaces = src->numfaces;

    VectorCopy( src->mins, dst->mins );
    VectorCopy( src->maxs, dst->maxs );
}

bool BrushAddFace( brush_t *b, brushface_t face )
{
    if (!b || !b->faces || (b->numfaces >= MAX_BRUSH_FACES))
    {
        return false;
    }
    if (b->numfaces >= b->maxfaces )
    {
        int newcap = b->maxfaces * 2;
        brushface_t* faces = realloc(b->faces, sizeof(brushface_t) * newcap );
        if (!faces) return false;
        b->faces = faces;
        b->maxfaces = newcap;
    }
    b->faces[b->numfaces++] = face;
    return true;
}
