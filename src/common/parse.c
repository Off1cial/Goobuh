#include "common/parse.h"
#include "common/common.h"
#include <stdlib.h>
#include <ctype.h>

parse_t gParse = {0};



FORCEINLINE int at_end( void )
{
    return (!gParse.c) || *gParse.c == '\0';
}

FORCEINLINE int is_c( char c )
{
    return !at_end() && (*gParse.c == c);
}

FORCEINLINE bool march_to( char c )
{
    while (!at_end())
    {
        if (*gParse.c == c) return true;
        gParse.c++;
    }
    return false;
}

void Parse_SkipLine( void )
{
    while (!at_end() && *gParse.c != '\n')
        gParse.c++;
}

bool Parse_AtEnd( void )
{
    return at_end();
}

bool Parse_SkipTo( char c )
{
    return march_to( c );
}

bool Parse_SetBuffer( char *buff )
{
    if (!buff) return false;
    gParse.c = buff;
    return true;
}


void Parse_SkipWhiteSpace( void )
{
    while (!at_end() && isspace((unsigned char)*gParse.c))
        gParse.c++;
}

bool Parse_ReadString( char *out )
{
    if (!march_to('"')) return false;
    gParse.c++; // Now at the start of the string
    
    int n = 0;
    while (!is_c('"'))
    {
        out[n++] = *gParse.c++;
    }
    out[n] = '\0';
    if (at_end()) return false;
    gParse.c++;
    return true;
}

bool Parse_ReadInt( int *out )
{
    Parse_SkipWhiteSpace();
    

    char *end;
    long result = strtol( gParse.c, &end, 10 );

    if (end == gParse.c )
    {
        return  false;
    }

    gParse.c = end;
    *out = (int)result;
    return true;
}

bool Parse_ReadU8( byte *out )
{
    int val;
    if (!Parse_ReadInt( &val)) return false;
    *out = (byte)val;
    return true;
}

bool Parse_ReadFloat( float *out )
{
    Parse_SkipWhiteSpace();

    char* end;
    float res = strtof( gParse.c, &end );
    if (end == gParse.c) return false;
    gParse.c = end;
    *out = res;
    return true;
}



bool Parse_FindLiteral( const char *str )
{
    if (!str) return false;
    long len = strlen( str );
    while (!at_end())
    {
        if (strncmp( gParse.c, str, len ) == 0)
        {
            gParse.c += len;
            return true;
        }
        gParse.c++; 
    }
    return false;
}

bool Parse_ReadVec3( float out[3] )
{
    Parse_SkipWhiteSpace();
    for (int i = 0; i < 3; i++)
    {
        if (!Parse_ReadFloat( &out[i] )) return false;
    }
    return true;
}
