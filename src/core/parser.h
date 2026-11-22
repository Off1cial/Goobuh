#ifndef __PARSER_H__
#define __PARSER_H__

#include "core/defs.h"

// Token types
#define TT_STRING 1
#define TT_NUMBER 2
#define TT_PUNCTUATION 3

// Number sub-types
#define TT_DECIMAL 0x1
#define TT_HEX     0x2
#define TT_OCTAL   0x4

#define TT_FLOAT   0x8
#define TT_SINGLE_PRECISION 0x10
#define TT_DOUBLE_PRECISION 0x20

#define TT_IPADDRESS 0x40

class Token
{
public:
    u32 type;
    u32 subtype;

    int GetIntValue();
    float GetFloatValue();
    double GetDoubleValue();
private:
    Token *next;
}; 

class Vector;

// Doesnt use tokens yet, does not need to yet but it will be helpful for:
// Custom shaders, custom scripting, etc
class Parser
{
public:
    // OO these are a bit ambiguous
    Parser (const char* filepath, bool isOSPath = false);
    Parser (const char* buff, int length);
    Parser();
    ~Parser();
    

    // The parser is responsible for handling the FILE*
    bool  LoadFile (const char* path, bool isOSPath = false);
    // Expects null termination
    bool  LoadMemory (const char* s, int length);
    // Read no more than N bytes of a "sample" into out

    bool ReadString (char *out, int n);
    bool ReadInt (int *out);
    bool ReadFloat (float *out);

    bool ReadVector (Vector &out);
    
private:
    char *m_current; // Pointer into buffer
    char *m_buff; // Allocated region of characters
    FILE *m_fptr;
    int   m_length;
    bool  m_loaded;
    
    void Init();

    // Release current file or buffer
    void Release();
    FORCEINLINE bool March (char c);
    FORCEINLINE bool AtEnd() const;
    FORCEINLINE void SkipWhitespace();
};

#endif
