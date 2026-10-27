#include "engine/map/bsp.h"
#include "common/common.h"
#include "engine/system/window.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

static float axial_bonus = 1.2f;
#define AXIAL_EPS 0.01f
#define SPLIT_EPS 0.01f

bspnode_t dnodes[MAX_BSP_NODES];
int numnodes = 0;

int dleafs[MAX_BSP_NODES];
int numleaves = 0;


plane_t dplanes[MAX_BRUSH_FACES * MAX_BRUSHES];
int numplanes = 0;

brushface_t dfaces[ MAX_BRUSH_FACES * MAX_BRUSHES ];
int numfaces;


brush_t* dbrushes = NULL;

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

int FindPlane( plane_t p )
{
    for (int i = 0; i < numplanes; i++)
    {
        if (PlaneEqual( dplanes[i], p )) return i;
    }
    dplanes[numplanes] = p;
    return numplanes++;
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

static void PushBrush( brush_t **head, brush_t *b )
{
  b->prev = NULL;
  b->next = *head;
  if (*head) (*head)->prev = b;
  *head = b;
}


static bool BrushValid( const brush_t *b )
{
    if (!b || b->numfaces < 4) return false;
    int good = 0;
    for (int f = 0; f < b->numfaces; f++)
        if (b->faces[f].win.numpoints >= 3) good++;
    return good >= 4;
}

static void FreeBrushList( brush_t* list )
{
  while(list) {brush_t* b = list->next; FreeBrush(list); list = b;}
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
            plane_t splane = dplanes[pn];
            Con_printf("Plane {%0.2f, %0.2f, %0.2f}, %0.2f\n", splane.normal[0], splane.normal[1], splane.normal[2], splane.d );
            float s = score_plane( dplanes[pn], list );
            Con_printf("Plane score: %0.2f\n", s );
            if (s < best) { best = s; best_plane = pn; }
        }
    return best_plane;       // -1 = nothing useful left, make a leaf
}





static int BuildTree_r( brush_t* list )
{
  if (!list) return CONTENTS_EMPTY;
  
  int planenum = choose_plane( list );
  
  if (planenum == -1 )
  {
    // Make a leaf
    int idx = numnodes++;
    bspnode_t *leaf = &dnodes[idx];
    leaf->planenum = planenum;
    leaf->firstface = numfaces;
    leaf->numfaces = 0;
    // Collect brush faces
    for (brush_t *b = list; b ; b = b->next )
    {
      for (int f = 0 ; f < b->numfaces; f++)
      {
        dfaces[numfaces++] = b->faces[f];
        leaf->numfaces++;
      }
    }
    FreeBrushList( list );
  
    dleafs[numleaves++] = idx;

    return idx;

  }

  int idx = numnodes++;
  dnodes[idx].planenum = planenum;


  brush_t* left, *right = NULL;
  brush_t* next;

  for (brush_t *b = list; b ; b = next )
  {
    next = b->next;

    int class = ClassifyBrush( b, dplanes[planenum] ); 

    if ( class == (SPLIT_FRONT | SPLIT_BACK ))
    {
      brush_t* fbrush = AllocBrush( b->numfaces );
      brush_t* bbrush = AllocBrush( b->numfaces );
      CSG_SplitBrush( b, dplanes[planenum], bbrush, fbrush );
      if( BrushValid( fbrush ) ) PushBrush( &left, fbrush );
      else FreeBrush( fbrush );
      if( BrushValid( bbrush ) ) PushBrush( &right, bbrush );
      else FreeBrush( bbrush );
      FreeBrush(b);
    }
    else if (class & SPLIT_FRONT ){ 
      if (BrushValid( b ))
      {
        PushBrush( &left, b ); 
      }
      else{
        FreeBrush(b);
      }
    }
    else PushBrush( &right, b );
  }

  int cl = BuildTree_r( left );
  int cr = BuildTree_r( right );

  dnodes[idx].children[0] = cl;
  dnodes[idx].children[1] = cr;
  return idx;
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


void BSP_DumpDot( const char *path )
{
    FILE *f = fopen( path, "w" );
    if (!f) return;

    fprintf( f, "digraph bsp {\n  node [shape=box, fontname=monospace];\n" );

    for (int i = 0; i < numnodes; i++)
    {
        const bspnode_t *n = &dnodes[i];

        if (n->planenum < 0)    // face leaf (from the rewritten BuildTree_r)
        {
            fprintf( f, "  n%d [shape=ellipse, style=filled, fillcolor=lightgrey, "
                        "label=\"leaf %d\\n%d faces\"];\n", i, i, n->numfaces );
            continue;
        }

        const plane_t *p = &dplanes[n->planenum];
        fprintf( f, "  n%d [label=\"node %d\\n(%.2f %.2f %.2f)\\nd=%.2f\"];\n",
                 i, i, p->normal[0], p->normal[1], p->normal[2], p->d );

        for (int c = 0; c < 2; c++)
        {
            const char *side = c == 0 ? "front" : "back";
            int ch = n->children[c];

            if (ch >= 0)
                fprintf( f, "  n%d -> n%d [label=\"%s\"];\n", i, ch, side );
            else
            {
                // contents leaf (negative child): give each its own box
                fprintf( f, "  c%d_%d [shape=ellipse, style=filled, fillcolor=%s, label=\"%s\"];\n",
                         i, c, ch == CONTENTS_SOLID ? "salmon" : "lightblue",
                         ch == CONTENTS_SOLID ? "SOLID" : "EMPTY" );
                fprintf( f, "  n%d -> c%d_%d [label=\"%s\"];\n", i, i, c, side );
            }
        }
    }

    fprintf( f, "}\n" );
    fclose( f );
}

static void ComputeNodeBounds( void )
{

    for (int i = 0; i < numnodes; i++) 
    {
        vec3_t mins, maxs;
        VectorSet( mins, FLT_MAX, FLT_MAX, FLT_MAX );
        VectorSet( maxs, -FLT_MAX, -FLT_MAX, -FLT_MAX );
        bspnode_t* node = &dnodes[i];
        if (node->numfaces <= 0) continue;

        for (int f = 0; f < node->numfaces; f++)
        {
            brushface_t* face = &dfaces[f + node->firstface];
            
            for (int v = 0; v < face->win.numpoints; v++)
            {
                float x, y, z;
                x = face->win.p[v][0];
                y = face->win.p[v][1];
                z = face->win.p[v][2];
                mins[0] = (x < mins[0]) ? x : mins[0];
                mins[1] = (y < mins[1]) ? y : mins[1];
                mins[2] = (z < mins[2]) ? z : mins[2];

                maxs[0] = (x > maxs[0]) ? x : maxs[0];
                maxs[1] = (y > maxs[1]) ? y : maxs[1];
                maxs[2] = (z > maxs[2]) ? z : maxs[2];
            }
        }
        VectorCopy( mins, node->mins );
        VectorCopy( maxs, node->maxs );
    }
}

#define NODESIZE ( sizeof(float) * 6 + (sizeof(int) * 5)  )

SZ_NEW( sz_bspwrite, END_LITTLE );

static bool write_node( bspnode_t *node, char *buff, int bufflen )
{ 
    if (bufflen < (int)NODESIZE) return false;

    return true;
}


static void WriteBSPFile( FILE* bsp_out )
{
    if (!bsp_out) return;

    u32 head = 0;
    
    char nodebuff[MAX_BSP_NODES * NODESIZE];
    for (int i = 0; i < numnodes; i++)
    {
        write_node(&dnodes[i], nodebuff + (i * NODESIZE), NODESIZE);
    }
    fwrite( nodebuff, numnodes * NODESIZE, 1, bsp_out);
}

#undef NODESIZE
void BSP_Run( FILE *bsp_file )
{
    if (!bsp_file) return; 

    //BSP_Subtract();
    int node = BuildTree_r( dbrushes );
    dbrushes = NULL;

    ComputeNodeBounds();
    //SZ_Alloc( &sz_bspwrite, 1024, MEMTAG_LIFE);
    //WriteBSPFile( bsp_file );

    BSP_DumpDot( "bsp.dot");
    Con_printf("BSP Finished: %d nodes, %d leaves\n", numnodes, numleaves );
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
        //Con_printf("Freeing brush\n");
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
