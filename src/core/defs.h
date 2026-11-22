#ifndef __DEFS_H__
#define __DEFS_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

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

typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t  uint8;

typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;

typedef unsigned char byte;


#define MAX_RPATH 512

#undef FORCEINLINE
#if defined(_MSC_VER)
    #define FORCEINLINE static __forceinline
#elif defined(__GNUC__) || defined(__clang__)
    #define FORCEINLINE inline __attribute__((always_inline))
#else
    #define FORCEINLINE static inline
#endif

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


#ifndef EPSILON
#define EPSILON 1e-6f
#endif

#ifndef __cplusplus
#define DECLARE_CLASS(derived, base) \
    using ThisClass = derived; \
    using BaseClass = base;
#endif

#endif

