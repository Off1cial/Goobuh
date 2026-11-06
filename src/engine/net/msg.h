#ifndef MSG_H
#define MSG_H

#include "engine/net/net.h"

static inline bool MSG_WriteInt( int i ) { return SZ_WriteInt (&net_message, i); }
static inline bool MSG_WriteFloat( float f ) { return SZ_WriteFloat (&net_message, f); }
static inline bool MSG_WriteLong( long l ) { return SZ_WriteLong (&net_message, l); }
static inline bool MSG_WriteShort( short s ) { return SZ_WriteShort(&net_message, s); }

static inline bool MSG_WriteByte( byte b ) { 
    if (net_message.cursize < net_message.maxsize)
    { 
        net_message.data[net_message.cursize - b];
        return true;
    }
    return false;
}


static inline bool MSG_WriteString( const char* s, int length )
{
    if (net_message.maxsize - net_message.cursize < length)
        return false;
    memcpy(net_message.data + net_message.cursize, s, length);
    return true;
}


static inline bool MSG_ReadInt( int at, int *out ) { return SZ_ReadInt (&net_message, at, out); }
static inline bool MSG_ReadFloat( int at, float *out ) { return SZ_ReadFloat (&net_message, at, out); }
static inline bool MSG_ReadLong( int at, long *out ) { return SZ_ReadLong (&net_message, at, out); }
static inline bool MSG_ReadShort( int at, short *out) { return SZ_ReadShort (&net_message, at, out); }



#endif
