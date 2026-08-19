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
};



// Attempting to be mindful of creating copies on the stack via operator
FORCEINLINE void VectorAdd(const Vector &a, const Vector &b, Vector &out)
{
  out.x = a.x + b.x;
  out.y = a.y + b.y;
  out.z = a.z + b.z;
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
    out.x = a.y * b.z - a.z * b.y;
    out.y = a.z * b.x - a.x * b.z;
    out.z = a.x * b.y - a.y * b.x;
}

FORCEINLINE float VectorCrossNorm(const Vector& a, const Vector& b, Vector& out)
{
  VectorCross(a, b, out);
  return out.NormaliseInPlace();
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

// Fills 'out' with the minimum x,y,z of A and B
FORCEINLINE void VectorMin(const Vector& a, const Vector& b, Vector& out)
{
  out.x = Min(a.x, b.x);
  out.y = Min(a.y, b.y);
  out.z = Min(a.z, b.z);
}

FORCEINLINE void VectorMax(const Vector& a, const Vector& b, Vector& out)
{
  out.x = Max(a.x, b.x);
  out.y = Max(a.y, b.y);
  out.z = Max(a.z, b.z);
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


FORCEINLINE float VectorNormalise(Vector& v)
{
  float lensq = v.Length2();
  float invlen =  1.0f / sqrtf(lensq);
  v.x *= invlen;
  v.y *= invlen;
  v.z *= invlen;
  return lensq * invlen;
}

FORCEINLINE float Vector::NormaliseInPlace(void)
{
  return VectorNormalise(*this);
}

FORCEINLINE Vector Vector::Normalised(void) const
{
  Vector v = *this;
  VectorNormalise(v);
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

