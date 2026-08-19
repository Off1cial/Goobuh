#ifndef CORE_COMMON_H
#define CORE_COMMON_H

// Cross-platform (common)
#ifdef _MSC_VER
    #define FORCEINLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
    #define FORCEINLINE __attribute__((always_inline)) inline
#else
    #define FORCEINLINE inline
#endif

#endif