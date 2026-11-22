#ifndef __UTLFILE_H__
#define __UTLFILE_H__

#include "core/defs.h"
#include "core/util/utlmemory.h"


enum class CUtlFileMode
{
    r = 1, w = 2, a = 3, 
    rb = 4, wb = 5, ab = 6,
};

template< typename M = CUtlMemory<byte> >
class CUtlFile 
{
public:
    CUtlFile() {};
    // Expects literals or null-terminated buffers
    CUtlFile( const char* path, CUtlFileMode mode, bool isOSPath = false );

    bool Open( const char* path,  bool isOSPath = false );

    ~CUtlFile();

    bool ReadInto( void* destination, int length );

    #ifdef _DEBUG
    void DebugPrint();
    #endif

private:

    void ResolveRelativePath( const char* in, int32 in_len, char* out, int32 out_len );
    const char* AccessModeStr();

    int32 Length();

    CUtlFileMode m_eFileMode;
    FILE* m_pFile = nullptr;
    int32 m_iLength = 0;
    M m_pMemory;
};

template <typename M>
CUtlFile<M>::CUtlFile( const char *path, CUtlFileMode mode, bool isOSPath )
: m_eFileMode(mode)
{
    if (!isOSPath)
    {
        char resolvedpath[MAX_RPATH];
        ResolveRelativePath( path, strlen(path), resolvedpath, MAX_RPATH );
        m_pFile = fopen( resolvedpath, AccessModeStr() );
    }
    else
    {
        m_pFile = fopen( path, AccessModeStr() );
    }
    m_iLength = Length();
    Assert( nullptr != m_pFile );
}

template <typename M>
CUtlFile<M>::~CUtlFile()
{
    if (m_pFile)
    {
        fclose (m_pFile);
    }
    m_iLength = 0;
    m_pFile = nullptr;
}

template <typename M>
const char* CUtlFile<M>::AccessModeStr()
{
    switch (m_eFileMode)
    {
        case CUtlFileMode::wb: return "wb";
        case CUtlFileMode::rb: return "rb";
        case CUtlFileMode::r: return "r";
        case CUtlFileMode::w: return "w";
        case CUtlFileMode::ab: return "ab";
        case CUtlFileMode::a: return "a";
        default: return "0";
    }
}

template <typename M>
void CUtlFile<M>::ResolveRelativePath( const char* in, int32 in_len, char* out, int32 out_len )
{
    Assert( out_len >= sizeof(RESOURCE_DIR) + in_len);
    snprintf( out, out_len, "%s%s", RESOURCE_DIR, in );
}

template <typename M>
int32 CUtlFile<M>::Length()
{
    if (nullptr == m_pFile)
        return 0;
    
    fseek( m_pFile, 0, SEEK_END );
    int32 len = ftell( m_pFile );
    rewind( m_pFile );
    return len;
}

#ifdef _DEBUG
template <typename M>
void CUtlFile<M>::DebugPrint()
{
    printf("[CUtlFile Debug Print]\n");
    char line[256];
    while (fgets( line, 256, m_pFile ))
        printf("%s", line );
}
#endif

#endif
