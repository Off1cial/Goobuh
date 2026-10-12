#ifndef CORE_COMMON_H
#define CORE_COMMON_H

#include <string.h>
#include <stdint.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef int64_t  i64;
typedef int32_t  i32;
typedef int16_t  i16;
typedef int8_t   i8;

typedef unsigned char byte;

// Cross-platform
#ifdef _MSC_VER

    #ifdef FORCEINLINE
        #undef FORCEINLINE
    #endif

    #define FORCEINLINE __forceinline

#elif defined(__GNUC__) || defined(__clang__)
    #ifdef FORCEINLINE
        #undef FORCEINLINE
    #endif

    #define FORCEINLINE inline __attribute__((always_inline))

#else

    #ifdef FORCEINLINE
        #undef FORCEINLINE
    #endif

    #define FORCEINLINE inline

#endif

#define PARANOID

#define HUNK_SIZE (1024 * 1024 * 1024) // 1MB

#define UNITS2CM(units) ( (units) * 2 ) // 1 unit = 2cm
#define CM2UNITS(cm) ( (cm) * 0.5F ) // 1 cm = 0.5 units

// #define COMPILETIME_MAX and COMPILETIME_MIN for max/min in constant expressions
#define COMPILETIME_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define COMPILETIME_MAX(a, b) (((a) > (b)) ? (a) : (b))
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846F
#endif


#define DEG2RAD(theta) (theta * M_PI/180.0)
#define RAG2DEG(theta) (theta * 180.0/M_PI)


#define DECLARE( original, new ) typedef original new;


// Null-terminating strncpy from Quake III
static inline void Q_strncpy(char* dst, const char* src, size_t dstsize)
{
  strncpy(dst, src, dstsize - 1);
  dst[dstsize - 1] = '\0'; 
}

// Case-insensitive string compare upto n chars
int Q_stricmpn(const char* s1, const char* s2, int n);

// Case sensitive string compare to n chars
int Q_strncmp(const char* s1, const char* s2, int n);

// Case insensitive comparison of the entire string
int Q_stricmp(const char* s1, const char* s2);

#ifdef __cplusplus

  template <class T>
  T Clamp(T const &val, T const &minVal, T const &maxVal)
  {
    if (val < minVal)
      return minVal;
    else if (val > maxVal)
      return maxVal;
    else
      return val;
  }

  // This is the preferred Min operator. Using the MIN macro can lead to unexpected
  // side-effects or more expensive code.
  template <class T>
  T Min(T const &val1, T const &val2)
  {
    return val1 < val2 ? val1 : val2;
  }

  // This is the preferred Max operator. Using the MAX macro can lead to unexpected
  // side-effects or more expensive code.
  template <class T>
  T Max(T const &val1, T const &val2)
  {
    return val1 > val2 ? val1 : val2;
  }

#endif

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))


// Global control
extern bool is_client;
extern bool is_server;

typedef struct sizebuf_t
{ 
    byte    *data;
    int maxsize;
    int cursize;
} sizebuf_t;


void SZ_Alloc( sizebuf_t *buf, int startsize );
void SZ_Free( sizebuf_t *buf );
void SZ_Clear( sizebuf_t *buf );
// Return a pointer to a region of a buffer that has space for 'bytes' bytes
byte *SZ_GetSpace( sizebuf_t *buf, int bytes );

typedef enum memtag_t
{
    MEMTAG_LIFE, // Exists for the whole lifespan of the engine
    MEMTAG_CLIENT, // Exists for the lifespan of the networking client
    MEMTAG_SERVER, // Exists for the lifespan of the networking server
    MEMTAG_LEVEL, // Exists for the lifespan of the current level, wiped of level change
} memtag_t;  


typedef struct sysarena_t
{
    uint8_t* base;
    size_t offset;
    size_t reserved;
    size_t mark; // Like a border on the shop conveyors, we say free up to here
    const char* name; // Optional, for naming the lifespan - use string literal
    int tag; // Categorising the purpose/lifespan
} sysarena_t;


extern sysarena_t LifeArena;

void *Hunk_Alloc( size_t size );

void *Sys_ArenaAlloc( sysarena_t* arena, size_t size );

void Sys_ArenaMark( sysarena_t *arena );
void Sys_ArenaFreeToMark( sysarena_t *arena );


bool Sys_ArenaInit( sysarena_t* arena, size_t size, const char* name, memtag_t tag );
void Sys_ArenaDestroy( sysarena_t* arena );


void Sys_FreeTag( memtag_t tag );
void Sys_ResetTag( memtag_t tag );





void Sys_Exit( void );
void Sys_Error( char* fmt, ... );
// Something critical failed, provides a message box and closes the program
[[noreturn]] void Sys_Crash( const char* fmt, ... );

void Con_printf( char* fmt, ... );













#endif // CORE_COMMON_H
