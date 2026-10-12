#include "engine/system/allocator.h"


#include "common/common.h"

#include <stdlib.h>
#include <string.h>


sysarena_t** arenas = NULL;
size_t arena_cap = 4;
size_t arena_count = 0;


static void add_arena( sysarena_t* arena )
{
    size_t    i;

    if (!arenas){
        arenas = calloc(1, sizeof(sysarena_t*) * arena_cap );
    }

    if (arena_count >= arena_cap){
        arenas = realloc(arenas, sizeof(sysarena_t*) * arena_cap * 2 );
        if (!arenas) Sys_Crash("add_arena: failed to grow arena array\n");
        // clear new range
        memset( arenas + arena_cap, 0, sizeof(sysarena_t*) * arena_cap);
        arena_cap *= 2;
    }
    // Find a slot
    for (i = 0; i < arena_cap; i++)
    {
        if (!arenas[i]){
            arenas[i] = arena;
            break;
        }
    }
}

static void close_all( void )
{
    for (size_t i = 0; i < arena_cap; i++ )
    {
        if (arenas[i]) Sys_ArenaDestroy( arenas[i] );
        arenas[i] = NULL;
    }
}

bool Sys_ArenaInit( sysarena_t* arena, size_t size, const char* name, memtag_t tag )
{
    size = MIN(size, 1024 * 1024 );

    if (!arena) return false;
    memset(arena, 0, sizeof(*arena));
    arena->name = (name != NULL) ? name : "Unnamed Arena";
    arena->reserved = size;
    arena->tag = tag;

    arena->base = calloc( 1, size );
    if (!arena->base)
        return false;

    add_arena( arena ) ;
    return true;
}

void Sys_ArenaDestroy( sysarena_t* arena )
{
    if (!arena) return;

    if (arena->base) free(arena->base);
    arena->base = NULL;
    arena_count--;
    // Is the string leaked?
    Con_printf("Freeing arena: %s\n", arena->name );
    memset(arena, 0, sizeof(*arena));
}



void Sys_ResetTag( memtag_t tag )
{
    size_t      i;
    for ( i = 0; i < arena_cap; i++ )
    {
        if ( (arenas[i]) && (arenas[i]->tag == (int)tag) )
        {
            arenas[i]->offset = 0;
        }
    }
}

void Sys_FreeTag( memtag_t tag )
{
    size_t      i;
    for ( i = 0; i < arena_cap; i++ )
    {
        if ( (arenas[i]) && (arenas[i]->tag == (int)tag) )
        {
            Sys_ArenaDestroy( arenas[i] );
            arenas[i] = NULL;
        }
    }
}


void* Sys_ArenaAllocAligned( sysarena_t* arena, size_t size, size_t alignment )
{
    uintptr_t current = (uintptr_t)arena->offset + (uintptr_t)arena->base;
    uintptr_t aligned = ( current + (alignment - 1 ) ) & ~(alignment - 1);

    uintptr_t padding = aligned - current;

    size_t new_offset = arena->offset + padding + size;
    if (new_offset > arena->reserved){
        // Implement growth?
        return NULL;
    }
    arena->offset = new_offset;
    return (void*)aligned;
}



void* Sys_ArenaAlloc( sysarena_t* arena, size_t size )
{
    // Generic alignment to allow any type
    return Sys_ArenaAllocAligned( arena, size, 2 * sizeof( size_t ) );
}

void Sys_ArenaShutdownAll( void )
{
    close_all();
}


void Sys_ArenaMark( sysarena_t *arena )
{
    arena->mark = arena->offset;
}


void Sys_ArenaFreeToMark( sysarena_t *arena )
{
    arena->offset = arena->mark;
    uintptr_t size = (uintptr_t)arena->offset - (uintptr_t)arena->base;
    memset((uintptr_t*)&arena->offset, 0, size );
}