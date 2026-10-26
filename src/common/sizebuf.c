#include "common/common.h"
#include <stdlib.h>


void SZ_Clear( sizebuf_t *buf )
{
    #ifdef PARANOID
    memset(buf->data, 0, buf->maxsize);
    #endif
    buf->cursize = 0;
}

// Return a pointer to a region of a buffer that has space for 'bytes' bytes
byte *SZ_GetSpace( sizebuf_t *buf, int bytes )
{
    int  i;
    for (i = buf->cursize; i < buf->maxsize; i++)
    {
        if (buf->maxsize - i >= bytes)
            return &buf->data[i];
    }
    return NULL;
}

sizebuf_t SZ_New( endianness_t endian_write )
{
    sizebuf_t newbuf = {.cursize = 0, .data = NULL, .maxsize = 0, .endian_write = endian_write};
    return newbuf;
}


bool SZ_Alloc( sizebuf_t *buf, int startsize, memtag_t tag )
{
    if (!buf) return false;
    switch(tag)
    {

        case MEMTAG_LIFE:
        {
            buf->data = Hunk_Alloc( startsize );
            break;
        }
        case MEMTAG_CLIENT:
        {
            buf->data = Sys_ArenaAlloc( &cl_arena, startsize );
            break;
        }
        case MEMTAG_SERVER:
        {
            buf->data = Sys_ArenaAlloc( &sv_arena, startsize );
            break;
        }
        default:
        {
            buf->data = calloc(1, startsize );
            break;
        }
    }
    if (!buf->data) return false;
    return true;
}
void SZ_Free( sizebuf_t *buf );
void SZ_Clear( sizebuf_t *buf );
// Return a pointer to a region of a buffer that has space for 'bytes' bytes
byte *SZ_GetSpace( sizebuf_t *buf, int bytes );
bool SZ_WriteInt( sizebuf_t *buf, int i );
bool SZ_WriteFloat( sizebuf_t *buf, float f );

bool SZ_ReadInt( sizebuf_t *buf, u32 at, int* out );
float SZ_ReadFloat( sizebuf_t* buf, u32 at, float *out );