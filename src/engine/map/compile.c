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

static bool OpenFile( char *file )
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
    char map_path[MAX_PATH];
    char bsp_path[MAX_PATH];
    
    snprintf(map_path, 64, "%s%s", ASSET_DIR, map );
    snprintf(bsp_path, 64, "%s%s", ASSET_DIR, bsp_out );

    if (!OpenFile( map_path ))
        return false;

    FILE *bspfile = fopen(bsp_path, "w");
    // Dont have to close the arena, manually, the engine does it for us
    Sys_ArenaInit(&compile_arena, sizeof(brush_t) * MAX_BRUSHES, "MapCompileArena", MEMTAG_CUSTOM);
    dbrushes = NULL;
    brush_t *tmp = AllocBrush(6);

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
