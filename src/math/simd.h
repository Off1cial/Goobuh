#ifndef __SIMDPROCESSOR_H__
#define __SIMDPROCESSOR_H__

#include "core/defs.h"
// A class that you can load data into and get your speedy result

#define CPUSSE2 0x1
#define CPUAVX 0x2
#define CPUAVX512 0X4

#define SIMDARGS float *a, float *b, float *dst, size_t n

class SIMDProcessor
{
public:
    SIMDProcessor();
    ~SIMDProcessor();

    // Inline?
    void Add (float *a, float *b, float *dst, size_t n);
    void Subtract ( SIMDARGS );
    void MultiplyAdd ( SIMDARGS );

    void memset (void* src, char c, size_t n);
    void memcpy (void* dst, void* src, size_t n);
private:

    int GetCapabilities();

    int m_capability = 0;
    float* m_line0, *m_line1;
};

#undef SIMDARGS

#endif
