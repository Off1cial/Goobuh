#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine/system/allocator.h"
#include "engine/map/compile.h"
#include "engine/map/bsp.h"
#include "common/common.h"
#include "common/parse.h"
#include "engine/poly.h"

char* map_file = NULL;

plane_t dplanes[MAX_BRUSH_FACES * MAX_BRUSHES];
int numplanes = 0;



brush_t* dbrushes = NULL;

sysarena_t compile_arena;

void BuildAllBrushes( void );
void BuildBrush( brush_t* brush );

/*
static bool read_face( brushface_t *face )
{
    if (numplanes >= MAX_BRUSHES * MAX_BRUSH_FACES) return false;
        
    Parse_SkipWhiteSpace();
    if (gParse.c[0] != '(') return false;
    gParse.c++;

    plane_t plane = {0};
    Parse_ReadVec3( plane.normal );
    Parse_SkipWhiteSpace();
    if (gParse.c[0] == ')') gParse.c++;
    if (!Parse_ReadFloat( &plane.d )) return false;

    face->planenum = numplanes;
    dplanes[numplanes++] = plane;

    return true;
}

bool ReadBrush( brush_t* out )
{
    if (!out) return false;

    Parse_SkipWhiteSpace();
    if (Parse_AtEnd()) return false;

    if (gParse.c[0] != 'B' || gParse.c[1] != ':') return false;
    
    gParse.c+=2;

    int facecount = 0;
    if (!Parse_ReadInt( &facecount ) || facecount <= 0 || (facecount  > MAX_BRUSH_FACES) ) return false;
    
    Parse_SkipWhiteSpace();
    if (gParse.c[0] != '{') return false;
    gParse.c++;
    Parse_SkipWhiteSpace();

    for (int i = 0; i < facecount; i++)
    {
        brushface_t face = {0};
        face.win = (winding_t){0};
        if (!read_face( &face )) return false;
        if (!BrushAddFace( out, face )) return false;
    }
    
    Parse_SkipWhiteSpace();
    if (gParse.c[0] == '}') gParse.c++;


    Con_printf("Read brush of %d faces\n", out->numfaces);
    return true;
}
*/

/*  NEW FOR TRENCHBROOM */


#include <math.h>

static void skip_ws_comments( void )
{
    for (;;)
    {
        Parse_SkipWhiteSpace();
        if (gParse.c[0] == '/' && gParse.c[1] == '/')
            while (*gParse.c && *gParse.c != '\n') gParse.c++;
        else
            break;
    }
}

static bool read_point( double p[3] )
{
    skip_ws_comments();
    if (gParse.c[0] != '(') return false;
    gParse.c++;
    for (int i = 0; i < 3; i++)
    {
        char* end;
        p[i] = strtod( gParse.c, &end );
        if (end == gParse.c) return false;
        gParse.c = end;
    }
    skip_ws_comments();
    if (gParse.c[0] != ')') return false;
    gParse.c++;
    return true;
}

static bool read_face( brushface_t* face )
{
    if (numplanes >= MAX_BRUSHES * MAX_BRUSH_FACES) return false;

    double p1[3], p2[3], p3[3];
    if (!read_point( p1 ) || !read_point( p2 ) || !read_point( p3 )) return false;

    // skip texture name, offsets, scale etc. to end of line
    while (*gParse.c && *gParse.c != '\n') gParse.c++;

    // normal = cross(p1 - p2, p3 - p2)
    double a[3] = { p1[0] - p2[0], p1[1] - p2[1], p1[2] - p2[2] };
    double b[3] = { p3[0] - p2[0], p3[1] - p2[1], p3[2] - p2[2] };
    double n[3] = { a[1] * b[2] - a[2] * b[1],
                    a[2] * b[0] - a[0] * b[2],
                    a[0] * b[1] - a[1] * b[0] };
    double len = sqrt( n[0] * n[0] + n[1] * n[1] + n[2] * n[2] );
    if (len < 1e-9) return false;

    plane_t plane = { 0 };
    for (int i = 0; i < 3; i++)
    {
        n[i] /= len;
        if (fabs( n[i] ) < 1e-6)       n[i] = 0;   // snap axial planes
        else if (fabs( n[i] - 1 ) < 1e-6)  n[i] = 1;
        else if (fabs( n[i] + 1 ) < 1e-6)  n[i] = -1;
        plane.normal[i] = (float)n[i];
    }
    plane.d = (float)(n[0] * p1[0] + n[1] * p1[1] + n[2] * p1[2]);
    face->planenum = numplanes;
    dplanes[numplanes++] = plane;
    return true;
}

bool ReadBrush( brush_t* out )
{
    if (!out) return false;

    // find the next brush, skipping entity wrappers and key/value lines
    for (;;)
    {
        skip_ws_comments();
        char c = gParse.c[0];
        if (c == 0) return false;
        if (c == '"') { while (*gParse.c && *gParse.c != '\n') gParse.c++; continue; }
        if (c == '}') { gParse.c++; continue; }
        if (c == '{')
        {
            gParse.c++;
            skip_ws_comments();
            if (gParse.c[0] == '(') break;   // it's a brush
            continue;                        // it's an entity
        }
        return false;
    }

    for (;;)
    {
        skip_ws_comments();
        if (gParse.c[0] == '}') { gParse.c++; break; }
        brushface_t face = { 0 };
        face.win = (winding_t){ 0 };
        if (!read_face( &face )) return false;
        if (!BrushAddFace( out, face )) return false;
    }

    Con_printf( "Read brush of %d faces\n", out->numfaces );
    return true;
}

static bool open_file( char *file )
{
    FILE *fptr = fopen( file, "rb" );
    if (!fptr)
    {
        Con_printf("Failed to open map file: %s\n", file);
        return false;
    }
    long len = 0;

    fseek( fptr, 0, SEEK_END );
    len = ftell( fptr );
    rewind( fptr );

    map_file = malloc( len + 1 );
    if (!map_file)
    {
        Con_printf("Failed to allocate memory for map file: %s\n", file);
        fclose(fptr);
        return false;
    }
    fread( map_file, 1, len, fptr );
    map_file[len] = 0; // null terminate
    fclose(fptr);

    gParse.c = map_file;
    
    return true;
}

static void CloseFile( void )
{
    if (map_file) free(map_file);
    gParse.c = NULL;
    map_file = NULL;
}

bool MAP_Compile( const char *map, const char *bsp_out )
{
    char map_path[MAX_QPATH];
    char bsp_path[MAX_QPATH];
    
    snprintf(map_path, sizeof(map_path), "%s%s", ASSET_DIR, map );
    snprintf(bsp_path, sizeof(bsp_path), "%s%s", ASSET_DIR, bsp_out );

    if (!open_file( map_path ))
        return false;

    FILE *bspfile = fopen(bsp_path, "w");
    if (!bspfile)
    {
        Con_printf("Failed to open bsp file\n");
        return false;
    }
    fprintf(bspfile, "i love onion rings");
    // Dont have to close the arena, manually, the engine does it for us
    Sys_ArenaInit(&compile_arena, sizeof(brush_t) * MAX_BRUSHES, "MapCompileArena", MEMTAG_CUSTOM);
    dbrushes = NULL;
    brush_t *tmp = AllocBrush( MAX_BRUSH_FACES );

    while (ReadBrush( tmp ))
    {
        brush_t* newbrush = AllocBrush( tmp->numfaces );
        if (!newbrush) return false;

        BrushCopy( newbrush, tmp );
        tmp->numfaces = 0;

        newbrush->next = dbrushes;
        newbrush->prev = NULL;

        if(dbrushes)
        {
            dbrushes->prev = newbrush;
        }
        dbrushes = newbrush;

    }
    CloseFile();
    BuildAllBrushes();
    FreeBrush(tmp);


    BSP_Run( bspfile );

    FreeAllBrushes();


    return true;
}



void BuildAllBrushes( void )
{
    brush_t* brush;
    for (brush = dbrushes; brush; brush = brush->next )
    {
        BuildBrush( brush );
    }
}


static void build_face( brush_t* brush, int f )
{
    brushface_t* face = &brush->faces[f];
    PlaneWinding( dplanes[face->planenum], &face->win);
    for (int other = 0; other < brush->numfaces; other++)
    {
        int planenum = brush->faces[other].planenum;
        winding_t out;
        if (other == f) continue;
        ClipWinding( &face->win, dplanes[planenum], &out );
        face->win = out;
        if (face->win.numpoints < 3 ) { face->win.numpoints = 0; return; }
    }
}


void BuildBrush( brush_t *brush )
{
    for (int f = 0; f < brush->numfaces; f++)
        build_face( brush, f );

    int n = 0;
    for ( int f = 0; f < brush->numfaces; f++)
    {
        if (brush->faces[f].win.numpoints >= 3)
        {
            brush->faces[n++] = brush->faces[f];
        }

    }
    brush->numfaces = n;
    // Compute mins/maxs
    VectorSet( brush->mins, FLT_MAX, FLT_MAX, FLT_MAX );
    VectorSet( brush->maxs, -FLT_MAX, -FLT_MAX, -FLT_MAX );
    
    for ( int f = 0; f < brush->numfaces; f++)
    {
        brushface_t* face = &brush->faces[f];
        for (int numverts = 0; numverts < face->win.numpoints; numverts++)
        {
            for (int i = 0; i < 3; i++)
            {
                float x = face->win.p[numverts][i];
                if (x < brush->mins[i]) brush->mins[i] = x;
                if (x > brush->maxs[i]) brush->maxs[i] = x;
            }
        }
    }
}
