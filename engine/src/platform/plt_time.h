#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint64_t plt_tiemmicros(void);
uint64_t plt_timemillis(void);

#ifdef __cplusplus
}
#endif