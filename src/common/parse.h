#ifndef PARSE_H
#define PARSE_H

#include "common/common.h"
#include <stdio.h>

typedef struct parse_t
{
    char* c;
} parse_t;

extern parse_t gParse;

bool Parse_SetBuffer( char *file );
void Parse_SkipWhiteSpace( void );
void Parse_SkipLine( void );
bool Parse_SkipTo( char c );
bool Parse_AtEnd( void );

bool Parse_FindLiteral( const char *str );

bool Parse_ReadU8( byte *out );
bool Parse_ReadInt( int *out );
bool Parse_ReadFloat( float *out );
bool Parse_ReadString( char *out );

bool Parse_ReadVec3( float out[3] );
 
#endif
