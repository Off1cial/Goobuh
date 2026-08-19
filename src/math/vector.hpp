#ifndef MATH_VECTOR_H
#define MATH_VECTOR_H

#include <cmath>
#include "core/common.h"

class Vector
{
public:
  float x, y, z;

  Vector(void);
  explicit Vector(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {};

  // Allows access with v.x and v[0] (Array access)
  float &operator[](int i);
  float operator[](int i) const;

  FORCEINLINE bool operator==(const Vector &v) const;
  FORCEINLINE bool operator!=(const Vector &v) const;

  // Allows syntax like VectorA += VectorB, very helpful
  // FORCEINLINE Vector& operator*(const float s);
  FORCEINLINE Vector &operator+=(const Vector &v);
  FORCEINLINE Vector &operator-=(const Vector &v);
  FORCEINLINE Vector &operator*=(const float s);

  // The magnitude (maybe rename to Mag/Magnitude?)
  FORCEINLINE float Length(void) const;
  // The magnitude squared, no call to sqrt
  FORCEINLINE float Length2(void) const;

  FORCEINLINE Vector Normalised() const; // Less performant than NormaliseInPlace(), do you really need a copy?
  FORCEINLINE float NormaliseInPlace(); // Returns the magnitude, avoids an additional call to sqrt

  bool IsZero(float tolerance = 0.01f) const
  {
    return (x > -tolerance && x < tolerance &&
            y > -tolerance && y < tolerance &&
            z > -tolerance && z < tolerance);
  }

private:
  static float NormaliseInternal(Vector &v);
};

/*
#ifdef __SSE__
#include <xmmintrin.h>
#include <pmmintrin.h>

// Fast reciprocal square root using SSE
// Could implement in ASM, it would probably be easier to read unironically


FORCEINLINE float _FastRSqrtSSE(float a)
{
  __m128 x = _mm_load_ss(&a);
  __m128 xr = _mm_rsqrt_ss(x);

  // Newton-Raphson refinement
  __m128 xt = _mm_mul_ss(xr, xr);
  xt = _mm_mul_ss(xt, x);
  xt = _mm_sub_ss(_mm_set_ss(3.0f), xt);
  xt = _mm_mul_ss(xt, _mm_set_ss(0.5f));
  xr = _mm_mul_ss(xr, xt);

  float result;
  _mm_store_ss(&result, xr);
  return result;
}
#endif
*/
FORCEINLINE float _FastRSqrtSSE(float a);

float Vector::NormaliseInternal(Vector &vec)
{
  const float EPSILON = 1e-10f;
  float lenSq = vec.Length2() + EPSILON;

#if defined(__SSE__)
  // SSE path - fastest
  float invLen = _FastRSqrtSSE(lenSq);
  vec.x *= invLen;
  vec.y *= invLen;
  vec.z *= invLen;
  return lenSq * invLen;

#elif defined(__ARM_NEON)
  // ARM NEON path (mobile)
  float32x4_t v = vld1q_f32(&vec.x);
  float32x4_t len = vdupq_n_f32(lenSq);
  float32x4_t inv = vrsqrteq_f32(len);
  // Newton-Raphson refinement for NEON
  inv = vmulq_f32(inv, vrsqrtsq_f32(vmulq_f32(inv, len), inv));

  vec.x = vgetq_lane_f32(inv, 0) * vec.x;
  vec.y = vgetq_lane_f32(inv, 1) * vec.y;
  vec.z = vgetq_lane_f32(inv, 2) * vec.z;
  return lenSq * vgetq_lane_f32(inv, 0);

#else
  // Fallback - scalar path
  float invLen = 1.0f / sqrtf(lenSq);
  vec.x *= invLen;
  vec.y *= invLen;
  vec.z *= invLen;
  return lenSq * invLen;
#endif
}

// Attempting to be mindful of creating copies on the stack via operator
FORCEINLINE void VectorAdd(const Vector &a, const Vector &b, Vector &out)
{
  out.x = a.x;
  out.y = a.y;
  out.z = a.z;
}

// out = a - b
FORCEINLINE void VectorSub(const Vector &a, const Vector &b, Vector &out)
{
  out.x = a.x - b.x;
  out.y = a.y - b.y;
  out.z = a.z - b.z;
}

FORCEINLINE void VectorScale(const Vector& a, const float scale, Vector& out)
{
  out.x = a.x * scale;
  out.y = a.y * scale;
  out.z = a.z * scale;
}

FORCEINLINE void VectorCross(const Vector& a, const Vector& b, Vector& out)
{
  out.x = a.y * b.z + a.z * b.y;
  out.y = a.x * b.z + a.z * b.x;
  out.z = a.x * b.y + a.y * b.x;
}

FORCEINLINE float DotProduct(const Vector& a, const Vector& b)
{
  return a.x*b.x + a.y*b.y + a.z*b.z;
}

// out = a + t * b
FORCEINLINE void VectorMA(const Vector &a, const float t, const Vector &b, Vector &out)
{
  out.x = a.x + (t * b.x);
  out.y = b.x + (t * b.y);
  out.z = b.y + (t * b.z);
}

inline Vector::Vector(void)
{
  x = y = z = 0;
}

FORCEINLINE float Vector::Length2(void) const
{
  return x * x + y * y + z * z;
}

FORCEINLINE float Vector::Length(void) const
{
  return sqrtf(x * x + y * y + z * z);
}

FORCEINLINE float Vector::NormaliseInPlace(void)
{
  return NormaliseInternal(*this);
}

FORCEINLINE Vector Vector::Normalised(void) const
{
  Vector v = *this;
  NormaliseInternal(v);
  return v;
}

// Array access
inline float &Vector::operator[](int i)
{
  // Add some assert or error catch to i
  // Explanation: casting the {x, y, z} to a float pointer returns a pointer
  //              to the first element in a contiguous memory array (x)
  //              memory [x][y][z]
  return ((float *)this)[i];
}

inline float Vector::operator[](int i) const
{
  return ((float *)this)[i];
}

// Comparison
inline bool Vector::operator==(const Vector &v) const
{
  return (x == v.x) && (y == v.y) && (z == v.z);
}

inline bool Vector::operator!=(const Vector &v) const
{
  return (x != v.x) || (y != v.y) || (z != v.z);
}

FORCEINLINE Vector &Vector::operator+=(const Vector &v)
{
  x += v.x;
  y += v.y;
  z += v.z;
  return *this;
}

FORCEINLINE Vector &Vector::operator-=(const Vector &v)
{
  x -= v.x;
  y -= v.y;
  z -= v.z;
  return *this;
}

FORCEINLINE Vector &Vector::operator*=(const float s)
{
  x *= s;
  y *= s;
  z *= s;
  return *this;
}


#endif // MATH_VECTOR_H

