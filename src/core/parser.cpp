#include "core/parser.h"
#include "core/defs.h"
#include "math/vector.h"


// Handling



void Parser::Init()
{
    m_current = NULL;
    m_fptr = NULL;
    m_length = 0;
    m_loaded = false;
}

Parser::Parser (const char* filepath, bool isOSPath)
{
   Init(); 
   LoadFile (filepath, isOSPath);
}

Parser::Parser()
{
    Init();
}

Parser::~Parser()
{
    Release();
}

void Parser::Release()
{
    if (!m_loaded)
        return;
    if (m_fptr) 
    {
        fclose (m_fptr);
        m_fptr = NULL;
    }

    if (m_buff)
    {
        free (m_buff);
        m_buff = NULL;
    }



    m_current = NULL;
    m_length = 0;
    m_loaded = false;
}

bool Parser::LoadFile (const char* path, bool isOSPath)
{
    Release();
    FILE* fhandle = NULL;

    if (!isOSPath)
    {
        // Construct relative directory
        char rpath[MAX_RPATH]; 
        snprintf (rpath, MAX_RPATH, "%s%s", RESOURCE_DIR, path);
        fhandle = fopen (rpath, "rb");
    }
    else
    {
        fhandle = fopen (path, "rb");
    }

    // Check success
    if (!fhandle)
    {
        m_loaded = false;
        m_fptr = NULL;
        m_current = NULL;

        // Relay error
        return false;
    }
    // Determine length
    int srclength = 0;
    fseek (fhandle, 0, SEEK_END);
    srclength = ftell (fhandle);
    rewind (fhandle);
    // Allocate data from arena

    // Use heap directly for now
    m_buff = (char*)calloc (1, srclength + 1); 
    
    if (!m_buff)
    {
        // Raise allocation error
        
        fclose (fhandle);
        return false;
    }

    if (fread (m_buff, 1, srclength, fhandle) != (size_t)srclength)
    {
        free (m_buff);
        m_buff = NULL;
        fclose (fhandle);
        return false;
    }
    m_buff[srclength] = '\0';
    m_length = srclength; 
    m_current = m_buff;
    m_fptr = fhandle;
    m_loaded = true;

    return true;
}


// Parsing

FORCEINLINE bool Parser::AtEnd() const
{
    return (!m_current || (*m_current == '\0'));
}

FORCEINLINE bool Parser::March (char c)
{
    while (!AtEnd() && *m_current != c)
        m_current++;

    if (AtEnd())
        return false;
    m_current++;
    return true;
}

FORCEINLINE void Parser::SkipWhitespace()
{
    while (!AtEnd() && isspace(*(byte*)m_current))
        m_current++;
}



bool Parser::ReadInt (int* out)
{
    SkipWhitespace();
    char *end;
    long result = strtol (m_current, &end, 10);

    if (end == m_current)
        return false;
    
    m_current = end;
    *out = (int)result;
    return true;
}

bool Parser::ReadFloat (float *out)
{
    SkipWhitespace();
    char* end;
    float res = strtof (m_current, &end);

    if (end == m_current)
        return false;

    m_current = end;
    *out = res;
    return true;
}

// Could also use a static ring buffer approach
/*

static int slot = 0;
static char bufs[4][64];
read into bufs[slot % 4]
slot++;

*/
bool Parser::ReadString (char *out, int n)
{
    if (!March('"')) return false;
   
    int i = 0;
    while (i < n - 1 && !AtEnd() && *m_current != '"')
    {
        out[i++] = *m_current++;
    }
    // Null terminate the output
    out[i] = '\0';
    return March('"');
}

bool Parser::ReadVector (Vector &out)
{
    if (!March('('))
        return false;
    for (int i = 0; i < 3; i++)
    {
        if (!ReadFloat (&out[i]))
            return false;
    }
    return March(')');
}
