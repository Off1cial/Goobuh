#include "engine/rendering/model.h"
#include "engine/rendering/renderer.h"
#include "engine/rendering/gl.h"
#include "engine/exports.h"
#include "common/common.h"
#include "common/parse.h"
#include "common/hash.h"
#include <glad/glad.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>


/*
 *
 *
 *
 *  CHANGE THIS CODE TO USE ITS OWN MEMORY ARENA FROM THE ENGINE
 *
 *
 *
 *
 */

model_t gModels[MDL_BUCKETS];

model_load_t load_file( const char* full_path );

void MDL_Add( model_t *mdl )
{
    i16 index = Hash_String( mdl->name ) % MDL_BUCKETS;
    i16 into_bucket = 0; 
    model_t *first;
    for ( first = &gModels[index]; first->next; first = first->next )
    {
        into_bucket++;
    }
    mdl->key = (i32) ( (index << 16) | into_bucket );
    first->next = mdl;
    Con_printf("Model \'%s\' added: %d:%d\n", mdl->name, index, into_bucket);
}

qhandle_t MDL_Index( const char *name )
{
    i16 bucket = Hash_String( name ) % MDL_BUCKETS; 

    model_t *first = &gModels[bucket];
    while ( first )
    {
        if ( Q_stricmpn( name, first->name, 64 ) == 0) return first->key;
        first = first->next;
    }
    return QHANDLE_NONE;
}

model_t *MDL_FetchByName( const char *name )
{
    i16 bucket = Hash_String( name ) % MDL_BUCKETS; 

    model_t *first = &gModels[bucket];
    while ( first )
    {
        if ( Q_stricmpn( name, first->name, 64 ) == 0) return first;
        first = first->next;
    }
    return NULL;
}

model_t* MDL_Load( char *path )
{
    char resolved[MAX_PATH];
    snprintf( resolved, MAX_PATH, "%s%s", ASSET_DIR, path );
    
    model_load_t load = load_file( resolved );
    if (!load.vertices || (load.numvertices <= 0 )){
        printf("%d vertices at %p\n", load.numvertices, load.vertices ); 
        return NULL; 
    } 

    model_t* mdl = calloc(1, sizeof( model_t )); // Is this temporary version really necesssary, it seems to only exist for the name usage in the fail statement
    strncpy( mdl->name, load.name, 64 );
    mdl->name[63] = 0;


    GLsizeiptr vbytes = (GLsizeiptr)load.numvertices * sizeof(vertex_t);
    GLsizeiptr ibytes = (GLsizeiptr)load.numindices * sizeof(load.indices[0]);
    GLsizeiptr ioff   = (GLsizeiptr)r_ebo_offset * sizeof(load.indices[0]);
    if ( (r_vbo_bytes + vbytes > STATIC_VBO_SIZE) || (ioff + ibytes > STATIC_EBO_SIZE) )
    {
        Con_printf("Failed to load model \'%s\', static buffers are full!", mdl->name);
        if (load.vertices) free(load.vertices);
        if (load.indices) free(load.indices);
        free(mdl);
        return NULL;
    }

    mdl->index_offset = r_ebo_offset;
    mdl->vertex_offset = (unsigned int)r_vbo_bytes / sizeof(vertex_t);
    mdl->numindices = load.numindices;
    
    glBindVertexArray( r_vao );
    GL_UpdateBuffer( &r_vbo, r_vbo_bytes, vbytes, load.vertices );
    GL_UpdateBuffer( &r_ebo, ioff, ibytes, load.indices );
    r_vbo_bytes += vbytes;
    // Copy malloc'd indices into a hunk allocated array instead - automatic cleanup
    mdl->indices = Hunk_Alloc( ibytes );
    memcpy(mdl->indices, load.indices, ibytes );

    // Push vertices to the static vao...

    // Push indices to the static ebo...
    mdl->index_offset = r_ebo_offset;
    r_ebo_offset += load.numindices;

    // end
    if (load.vertices){ free(load.vertices); load.vertices = NULL; } // vertices are now in the vbo, free them
    if (load.indices){ free(load.indices); load.indices = NULL;}
    Con_printf("Loaded model \'%s\'\n  %d vertices\n", mdl->name, load.numvertices );
        

    // Swap heap to hunk
    model_t *hunkmdl = Hunk_Alloc( sizeof(model_t) );
    memcpy(hunkmdl, mdl, sizeof(model_t));
    free(mdl);
    mdl = NULL;

    MDL_Add( hunkmdl );
    return hunkmdl;
}


typedef struct
{
    unsigned int *indices;
    int capacity;
    int count;
} indexgroup_t;


indexgroup_t new_indexgroup( int capacity )
{
    indexgroup_t group =
    {
        .indices = malloc( sizeof(unsigned int) * capacity ),
        .capacity = capacity,
        .count = 0,
    };
    return group;
}

void free_indexgroup( indexgroup_t *group )
{
    if (!group) return;
    if (group->indices)
    {
        free(group->indices);
        group->indices = NULL;
    }
    group->capacity = 0;
    group->count = 0;
}

bool add_index( indexgroup_t *group, unsigned int index )
{
    if (!group) return false;

    if (group->count >= group->capacity)
    {
        int new_cap = group->capacity * 2;
        unsigned int *new_inds = realloc( group->indices, sizeof(unsigned int) * new_cap );
        if (!new_inds)
        {
            free_indexgroup( group );
            return false;
        }
        group->indices = new_inds;
        group->capacity = new_cap;
    }
    group->indices[group->count++] = index;
    return true;
}


typedef struct 
{
    vertex_t* vertices;
    int capacity;
    int count;
} vertexgroup_t;


vertexgroup_t new_vertexgroup( int capacity )
{
    vertexgroup_t group = 
    {
        .vertices = malloc( sizeof(vertex_t) * capacity ),
        .capacity = capacity,
        .count = 0,
    };
    return group;
}

void free_vertexgroup( vertexgroup_t *group )
{
    if (group->vertices)
    {
        free(group->vertices);
        group->vertices = NULL;
    }
    group->capacity = 0;
    group->count = 0;
}

bool add_vertex( vertexgroup_t *group, vertex_t v )
{
    if ( group->count >= group->capacity )
    {
        vertex_t* new_data = realloc( group->vertices, sizeof(vertex_t) * group->capacity * 2 );
        if (!new_data)
        {
            // Allcation failed
            // If this failed during model load, we dont care about keeping our data, we dont want a partial model
            free_vertexgroup( group );
            return false;
        }
        group->vertices = new_data;
        group->capacity *= 2;
    }

    group->vertices[group->count++] = v;
    return true;
}

// Only reads pos currently
static inline bool read_vertex( vertex_t* out )
{
    // Search for vertex line
    for (;;)
    {
        Parse_SkipWhiteSpace();
        if (Parse_AtEnd()) return false;

        if (gParse.c[0] == 'v' && isspace(gParse.c[1]))
        {
            gParse.c++; // Into the whitespace between 'v' and  first float
            break;
        }
        // Theres something after the 'v', this line is invalid
        Parse_SkipLine();
    }
    // We've found our vertex line
    int i;
    for ( i = 0; i < 3; i++) // read pos
    {
        if (!Parse_ReadFloat( &out->pos[i])) return false;
    }
    for ( i = 0; i < 3; i++) // Read normal
    {
        if (!Parse_ReadFloat( &out->normal[i])) return false;
    }
    for ( i = 0; i < 2; i++) // Read uv
    {
        if (!Parse_ReadFloat( &out->uv[i])) return false;
    }
    for ( i = 0; i < 4; i++) // read colour
    {
        if (!Parse_ReadU8( &out->col[i] )) return false;
    }
    printf("Read vertex:\n");
    printf(VectorString(out->pos));
    printf(VectorString(out->normal));
    printf("(%0.2f, %0.2f)\n", out->uv[0], out->uv[1]);
    printf("%d %d %d %d\n", out->col[0], out->col[1], out->col[2], out->col[3]);
    

    return true;
}

static inline int read_indices( indexgroup_t *inds, unsigned int base )
{
    // search for index line
    for (;;)
    {
        Parse_SkipWhiteSpace();
        if (Parse_AtEnd()) return 0;

        if (gParse.c[0] == 'i' && isspace(gParse.c[1]))
        {
            gParse.c++;
            break;
        }
        Parse_SkipLine();
    }
    int count = 0;
    int index;
    while (Parse_ReadInt( &index ))
    {
        index += base;
        if (!add_index(inds, index )) return false;
        count++;
    }
    return count;
}

static inline bool read_face( vertexgroup_t *model_verts, indexgroup_t *model_inds )
{
    // Find face line
    int v_count;
    int i_count;
    for (;;)
    {
        if (gParse.c[0] == '{' || gParse.c[0] == '}')
            gParse.c++;
        Parse_SkipWhiteSpace();
        if (Parse_AtEnd()) return false;

        if ( (gParse.c[0] == 'F') && (gParse.c[1] == ':') ){
            // Add a feature so the face doesnt have to specify its vertex count
            gParse.c+=2;
            break;
        }
        Parse_SkipWhiteSpace();
    }
    if (!Parse_ReadInt(&v_count) || ( v_count <= 0 )) return false;
    unsigned int base = (unsigned int)model_verts->count;
    // Read data
    vertex_t v = {0};
    for (int i = 0; i < v_count; i++)
    {
        if (!read_vertex( &v )) return false;
        if (!add_vertex( model_verts, v )) return false;
    }
    i_count = read_indices( model_inds, base );
    Con_printf("Face read: %d vertices, %d indices\n", v_count, i_count);
    return i_count > 0;
    //return true;
}

model_load_t load_file ( const char* full_path )
{
    FILE *fptr;
    model_load_t load = {0};
    load.indices = NULL;
    load.vertices = NULL;

    fptr = fopen( full_path, "rb" );
    if (!fptr)
    {
        Con_printf( "Failed to open model file: %s\n", full_path );
        return load;
    }else{
        Con_printf("Found .mdl at %s\n", full_path );
    }


    // Determine file size
    long len;
    fseek( fptr, 0, SEEK_END );
    len = ftell( fptr );
    rewind(fptr);
    // Allocate buffer
    char *buf = malloc( len + 1 );
    // Read into buffer
    fread( buf, 1, len, fptr ); 
    buf[len] = 0; // null terminate
    gParse.c = buf;
    // Close file
    fclose( fptr );
    
    int cap = 12; 
    vertexgroup_t mdl_vertices = new_vertexgroup( cap );
    indexgroup_t mdl_indices = new_indexgroup( cap );
    
    // Parse the file
    Parse_ReadString( load.name );
    Con_printf("Reading model \"%s\"\n", load.name );

    while (read_face( &mdl_vertices, &mdl_indices )){
    }
    
    free(buf);
    gParse.c = NULL;
    
    // Copy data to load
    size_t vsize = sizeof(vertex_t) * mdl_vertices.count;
    load.vertices = malloc( vsize );
    memcpy( load.vertices, mdl_vertices.vertices, vsize);

    size_t isize = sizeof(unsigned int) * mdl_indices.count;
    load.indices = malloc( isize );
    memcpy( load.indices, mdl_indices.indices, isize );

    load.numvertices = mdl_vertices.count; 
    load.maxvertices = mdl_vertices.capacity;
    
    load.numindices = mdl_indices.count;
    load.maxindices = mdl_vertices.capacity;

    free_vertexgroup( &mdl_vertices );
    free_indexgroup( &mdl_indices );
    return load;
}
