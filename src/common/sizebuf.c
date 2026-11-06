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

bool SZ_WriteShort(sizebuf_t *buf, short s)
{
    if (!buf)
        return false;
    byte *dst = SZ_GetSpace( buf, 2 );
    if (!dst) 
        return false;

    switch(buf->endian_write)
    {
        case END_LITTLE:
            return WriteU16LE(dst, 2, (uint16_t)s);
        case END_BIG:
            return WriteU16BE(dst, 2, (uint16_t)s);
        default:
            buf->cursize -= 2;
            return false;
    }
}

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

bool SZ_WriteLong( sizebuf_t* buf, long l )
{
    if (!buf)
        return false;

    byte *dst = SZ_GetSpace(buf, 8);
    if (!dst)
        return false;

    switch (buf->endian_write)
    {
        case END_LITTLE:
            return WriteU64LE(dst, 8, (uint64_t)l);

        case END_BIG:
            return WriteU64BE(dst, 8, (uint64_t)l);

        default:
            buf->cursize -= 8;
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

bool SZ_WriteBytes( sizebuf_t *buf, const byte *data, int len )
{
    if (!buf || !data || len < 0)
        return false;

    byte *dst = SZ_GetSpace(buf, len);
    if (!dst)
        return false;

    memcpy(dst, data, len);
    buf->cursize += len;
    return true;

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

bool SZ_ReadShort( sizebuf_t* buf, u32 at, short *out )
{
    if (!buf || buf->data || !out || at > (u32)buf->cursize || buf->cursize - at < 2)
        return false;
    
    uint16_t bits;
    switch(buf->endian_write)
    {
        case END_LITTLE:
            if (!ReadU16LE(buf->data + at, 2, &bits))
                return false;
            break;
        case END_BIG:
            if (!ReadU16BE(buf->data + at, 2, &bits))
                return false;
            break;
        default:
            return false;
    }
    int16_t val;
    memcpy(&val, &bits, sizeof(val));
    *out = val;
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

bool SZ_ReadLong( sizebuf_t* buf, u32 at, long *out )
{
    if (!buf || !buf->data || !out ||
        at > (u32)buf->cursize ||
        buf->cursize - at < 8)
        return false;

    switch (buf->endian_write)
    {
        case END_LITTLE:
            return ReadU64LE(buf->data + at, 8, (uint64_t*)out);

        case END_BIG:
            return ReadU64BE(buf->data + at, 8, (uint64_t*)out);

        default:
            return false;
    }
}
