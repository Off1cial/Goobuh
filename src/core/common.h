#ifndef CORE_COMMON_H
#define CORE_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef int64_t  i64;
typedef int32_t  i32;
typedef int16_t  i16;
typedef int8_t   i8;

// Cross-platform (common)
#ifdef _MSC_VER // Windows C/C++ Compiler
#define FORCEINLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define FORCEINLINE __attribute__((always_inline)) inline
#else
#define FORCEINLINE inline
#endif

// #define COMPILETIME_MAX and COMPILETIME_MIN for max/min in constant expressions
#define COMPILETIME_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define COMPILETIME_MAX(a, b) (((a) > (b)) ? (a) : (b))
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

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


#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))


#endif
#endif // CORE_COMMON_H
