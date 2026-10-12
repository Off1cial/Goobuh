#include "common/common.h"


void SZ_Alloc( sizebuf_t *buf, int startsize )
{
    buf->data = Hunk_Alloc( startsize );
    buf->cursize = 0;
    buf->maxsize = startsize;
}

void SZ_Clear( sizebuf_t *buf )
{
    #ifdef PARANOID
    memset(buf->data, 0, buf->maxsize);
    #endif
    buf->cursize = 0;
}

// Return a pointer to a region of a buffer that has space for 'bytes' bytes
byte *SZ_GetSpace( sizebuf_t *buf, int bytes );