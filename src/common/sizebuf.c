#include "common/common.h"
#include <stdlib.h>
#include <string.h>

#define BUFHEAD(buf) ((buf)->data + (buf)->cursize)
#define BUFLEFT(buf) ((buf)->maxsize - (buf)->cursize)

void SZ_Clear(sizebuf_t *buf)
{
    #ifdef PARANOID
    memset(buf->data, 0, buf->maxsize);
    #endif
    buf->cursize = 0;
}

byte *SZ_GetSpace(sizebuf_t *buf, int bytes)
{
    if (!buf || !buf->data || bytes < 0 ||
        bytes > buf->maxsize - buf->cursize)
        return NULL;

    byte *space = BUFHEAD(buf);
    buf->cursize += bytes;
    return space;
}

sizebuf_t SZ_New(endianness_t endian_write)
{
    sizebuf_t newbuf = {
        .cursize = 0,
        .data = NULL,
        .maxsize = 0,
        .endian_write = endian_write
    };
    return newbuf;
}

bool SZ_Alloc(sizebuf_t *buf, int startsize, memtag_t tag)
{
    if (!buf || startsize <= 0)
        return false;

    switch (tag)
    {
        case MEMTAG_LIFE:
            buf->data = Hunk_Alloc(startsize);
            break;

        case MEMTAG_CLIENT:
            buf->data = Sys_ArenaAlloc(&cl_arena, startsize);
            break;

        case MEMTAG_SERVER:
            buf->data = Sys_ArenaAlloc(&sv_arena, startsize);
            break;

        default:
            buf->data = calloc(1, startsize);
            break;
    }

    if (!buf->data)
        return false;

    buf->maxsize = startsize;
    buf->cursize = 0;
    return true;
}

// Integer serialisation


// Buffer writes

bool SZ_WriteInt(sizebuf_t *buf, int i)
{
    if (!buf)
        return false;

    byte *dst = SZ_GetSpace(buf, 4);
    if (!dst)
        return false;

    switch (buf->endian_write)
    {
        case END_LITTLE:
            return WriteU32LE(dst, 4, (uint32_t)i);

        case END_BIG:
            return WriteU32BE(dst, 4, (uint32_t)i);

        default:
            buf->cursize -= 4;
            return false;
    }
}

bool SZ_WriteFloat(sizebuf_t *buf, float f)
{
    if (!buf)
        return false;

    byte *dst = SZ_GetSpace(buf, 4);
    if (!dst)
        return false;

    switch (buf->endian_write)
    {
        case END_LITTLE:
            return WriteF32LE(dst, 4, f);

        case END_BIG:
            return WriteF32BE(dst, 4, f);

        default:
            buf->cursize -= 4;
            return false;
    }
}

// Buffer reads

bool SZ_ReadInt(sizebuf_t *buf, u32 at, int *out)
{
    if (!buf || !buf->data || !out ||
        at > (u32)buf->cursize ||
        buf->cursize - at < 4)
        return false;

    uint32_t bits;

    switch (buf->endian_write)
    {
        case END_LITTLE:
            if (!ReadU32LE(buf->data + at, 4, &bits))
                return false;
            break;

        case END_BIG:
            if (!ReadU32BE(buf->data + at, 4, &bits))
                return false;
            break;

        default:
            return false;
    }

    int32_t value;
    memcpy(&value, &bits, sizeof(value));
    *out = value;
    return true;
}

bool SZ_ReadFloat(sizebuf_t *buf, u32 at, float *out)
{
    if (!buf || !buf->data || !out ||
        at > (u32)buf->cursize ||
        buf->cursize - at < 4)
        return false;

    switch (buf->endian_write)
    {
        case END_LITTLE:
            return ReadF32LE(buf->data + at, 4, out);

        case END_BIG:
            return ReadF32BE(buf->data + at, 4, out);

        default:
            return false;
    }
}
