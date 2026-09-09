// hash.h — shared by cvars, fonts, textures, anything else that wants a string->bucket table
#pragma once
#include <stdint.h>

#define FNV1A_OFF 2166136261u
#define FNV_PRIME 16777619u
#define INT_BYTE(number, nth) (( (number) >> (8*   (nth) )) & 0xff);



static inline uint32_t Hash_String(const char* str)
{
    uint32_t h = FNV1A_OFF; // FNV-1a offset basis
    while (*str) { h ^= (uint8_t)*str++; h *= FNV_PRIME; } // FNV prime
    return h;
}

static inline uint32_t Hash_Int(const int num)
{
  uint32_t h = FNV1A_OFF;
  //uint8_t bytes[4];
  for (int i = 0; i < 4; i++) 
  {
    int8_t byte = INT_BYTE(num, i); 
    h *= FNV_PRIME;
    h^=(uint32_t)byte;
  }
  return h;
}

// Always use this when indexing into a fixed-size bucket array —
// never index with the raw hash directly.
static inline uint32_t Hash_Bucket(uint32_t hash, uint32_t bucket_count)
{
    return hash % bucket_count;
}

