#ifndef MSG_H
#define MSG_H

#include "engine/net/net.h"

static inline bool MSG_WriteInt( int i ) { return SZ_WriteInt (&net_message, i); }
static inline bool MSG_WriteFloat( float f ) { return SZ_WriteFloat (&net_message, f); }
static inline bool MSG_WriteLong( long l ) { return SZ_WriteLong (&net_message, l); }

static inline bool MSG_ReadInt( int at, int *out ) { return SZ_ReadInt (&net_message, at, out); }
static inline bool MSG_ReadFloat( int at, float *out ) { return SZ_ReadFloat (&net_message, at, out); }
static inline bool MSG_ReadLong( int at, long *out ) { return SZ_ReadLong (&net_message, at, out); }

#endif