#include "platform/time.h"
#include <stdint.h>

#if defined(_WIN32)

#include <windows.h>

uint64_t plt_timemicros(void)
{
  static LARGE_INTEGER frequency;
  LARGE_INTEGER counter;

  if (frequency.QuadPart == 0)
    QueryPerformanceFrequency(&frequency);

  QueryPerformanceCounter(&counter);

  return (uint64_t)((counter.QuadPart * 1000000ULL) /
                    frequency.QuadPart);
}

#elif defined(__linux__)
#include <time.h>

uint64_t plt_timemicros(void)
{
  struct timespec ts;

  clock_gettime(CLOCK_MONOTONIC, &ts);

  return (uint64_t)ts.tv_sec * 1000000ULL +
         (uint64_t)ts.tv_nsec / 1000ULL;
}

#else

#error "Unsupported platform"

#endif

uint64_t plt_timemillis(void)
{
  return plt_timemicros() / 1000ULL;
}