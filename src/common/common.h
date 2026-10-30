#ifndef CORE_COMMON_H
#define CORE_COMMON_H

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// Load Windows headers FIRST so winnt.h uses its own FORCEINLINE.
// Ours is defined afterwards and replaces it.
#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
#endif

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef int64_t  i64;
typedef int32_t  i32;
typedef int16_t  i16;
typedef int8_t   i8;

typedef unsigned char byte;

typedef i32 qhandle_t;
#define QHANDLE_NONE -1

// Cross-platform
// Defined AFTER the Windows headers so winnt.h never sees our version.
#undef FORCEINLINE
#if defined(_MSC_VER)
    #define FORCEINLINE static __forceinline
#elif defined(__GNUC__) || defined(__clang__)
    #define FORCEINLINE static inline __attribute__((always_inline))
#else
    #define FORCEINLINE static inline
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

#define RAD 0.017453292519943295F
#define DEG 57.29577951308232F


#define DECLARE( original, new ) typedef original new;

#define MAX_QPATH 512
#define ASSET_LEN sizeof ASSET_DIR

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

typedef struct usercmd_t
{
    float angles[3];
    float forward;
    float side;
    float up;
} usercmd_t;




typedef enum memtag_t
{
    MEMTAG_LIFE, // Exists for the whole lifespan of the engine
    MEMTAG_CLIENT, // Exists for the lifespan of the networking client
    MEMTAG_SERVER, // Exists for the lifespan of the networking server
    MEMTAG_LEVEL, // Exists for the lifespan of the current level, wiped of level change
    MEMTAG_CUSTOM, // Custom allocation and destruction
    MEMTAG_HEAP, // no
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
// Think about threads...
void Hunk_Mark( void );
void Hunk_FreeToMark( void );

void *Sys_ArenaAlloc( sysarena_t* arena, size_t size );

void Sys_ArenaMark( sysarena_t *arena );
void Sys_ArenaFreeToMark( sysarena_t *arena );

static inline void Sys_ArenaReset( sysarena_t* arena )
{
    if (arena) arena->offset = 0;
}

bool Sys_ArenaInit( sysarena_t* arena, size_t size, const char* name, memtag_t tag );
void Sys_ArenaDestroy( sysarena_t* arena );


void Sys_FreeTag( memtag_t tag );
void Sys_ResetTag( memtag_t tag );





[[noreturn]] void Sys_Exit( void );
void Sys_Error( char* fmt, ... );
// Something critical failed, provides a message box and closes the program
[[noreturn]] void Sys_Crash( const char* fmt, ... );

void Con_printf( char* fmt, ... );


void Sys_Assert( bool condition, char *fmt, ... );


unsigned long Sys_TimeMillis( void );


typedef enum endianness_t
{
  END_LITTLE = 0,
  END_BIG = 1,
} endianness_t; // Endian + 1 % 2

typedef struct sizebuf_t
{ 
    const endianness_t endian_write;
    int maxsize;
    int cursize;
    byte    *data;
} sizebuf_t;
extern sysarena_t sv_arena;
extern sysarena_t cl_arena;

#define SZ_NEW( name, w_endian ) sizebuf_t name = {.cursize = 0, .maxsize = 0, .endian_write = w_endian, .data = NULL}

extern sizebuf_t cl_sizebuff;
extern sizebuf_t sv_sizebuff;

sizebuf_t SZ_New( endianness_t endian_write );
bool SZ_Alloc( sizebuf_t *buf, int startsize, memtag_t tag );
void SZ_Free( sizebuf_t *buf );
void SZ_Clear( sizebuf_t *buf );
// Return a pointer to a region of a buffer that has space for 'bytes' bytes
byte *SZ_GetSpace( sizebuf_t *buf, int bytes );
bool SZ_WriteInt( sizebuf_t *buf, int i );
bool SZ_WriteFloat( sizebuf_t *buf, float f );

bool SZ_ReadInt( sizebuf_t *buf, u32 at, int* out );
bool SZ_ReadFloat( sizebuf_t* buf, u32 at, float *out );


// Integer serialisation

static inline bool WriteU32LE(byte *buff, int len, uint32_t value)
{
    if (!buff || len < 4)
        return false;

    buff[0] = (byte)value;
    buff[1] = (byte)(value >> 8);
    buff[2] = (byte)(value >> 16);
    buff[3] = (byte)(value >> 24);
    return true;
}

static inline bool WriteU32BE(byte *buff, int len, uint32_t value)
{
    if (!buff || len < 4)
        return false;

    buff[0] = (byte)(value >> 24);
    buff[1] = (byte)(value >> 16);
    buff[2] = (byte)(value >> 8);
    buff[3] = (byte)value;
    return true;
}

static inline bool ReadU32LE(const byte *buff, int len, uint32_t *out)
{
    if (!buff || !out || len < 4)
        return false;

    *out = (uint32_t)buff[0]
         | ((uint32_t)buff[1] << 8)
         | ((uint32_t)buff[2] << 16)
         | ((uint32_t)buff[3] << 24);
    return true;
}

static inline bool ReadU32BE(const byte *buff, int len, uint32_t *out)
{
    if (!buff || !out || len < 4)
        return false;

    *out = ((uint32_t)buff[0] << 24)
         | ((uint32_t)buff[1] << 16)
         | ((uint32_t)buff[2] << 8)
         | (uint32_t)buff[3];
    return true;
}

// Float serialisation

static inline bool WriteF32LE(byte *buff, int len, float value)
{
    uint32_t bits;
    memcpy(&bits, &value, sizeof(bits));
    return WriteU32LE(buff, len, bits);
}

static inline bool WriteF32BE(byte *buff, int len, float value)
{
    uint32_t bits;
    memcpy(&bits, &value, sizeof(bits));
    return WriteU32BE(buff, len, bits);
}

static inline bool ReadF32LE(const byte *buff, int len, float *out)
{
    uint32_t bits;

    if (!out || !ReadU32LE(buff, len, &bits))
        return false;

    memcpy(out, &bits, sizeof(bits));
    return true;
}

static inline bool ReadF32BE(const byte *buff, int len, float *out)
{
    uint32_t bits;

    if (!out || !ReadU32BE(buff, len, &bits))
        return false;

    memcpy(out, &bits, sizeof(bits));
    return true;
}




#endif // CORE_COMMON_H
