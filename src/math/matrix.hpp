#ifndef MATH_MATRIX_H
#define MATH_MATRIX_H

#include "math/vector.hpp"

class Mat4
{
public:
  static Mat4 Identity();
  static Mat4 Translation(const Vector &position);
  static Mat4 Scale(const Vector &scale);
  static Mat4 RotationX(float radians);
  static Mat4 RotationY(float radians);
  static Mat4 RotationZ(float radians);

  static Mat4 LookAt(const Vector &eye, const Vector &centre, const Vector &up);

  static Mat4 Perspective(
      float fov,
      float aspect,
      float znear,
      float zfar);

  static Mat4 Orthographic(
      float left,
      float right,
      float bottom,
      float top,
      float znear,
      float zfar);

  Mat4 operator*(const Mat4 &b) const;

  Vector operator*(const Vector &v) const;

  Mat4 Transposed() const;
  Mat4 Inverse() const;

  float m[16];
};

#ifndef MATH_MATRIX_HPP
#define MATH_MATRIX_HPP

#include <cmath>
#include <cstring>
#include "math/vector.hpp"

// ============================================================================
// Mat4 (4x4 Matrix)
// ============================================================================

class Mat4
{
public:
  float m[16];

  Mat4() { memset(m, 0, sizeof(m)); }

  static Mat4 Identity();
  static Mat4 Translation(const Vector &position);
  static Mat4 Scale(const Vector &scale);
  static Mat4 RotationX(float radians);
  static Mat4 RotationY(float radians);
  static Mat4 RotationZ(float radians);
  static Mat4 LookAt(const Vector &eye, const Vector &centre, const Vector &up);
  static Mat4 Perspective(float fov, float aspect, float znear, float zfar);
  static Mat4 Orthographic(float left, float right, float bottom, float top, float znear, float zfar);

  Mat4 operator*(const Mat4 &b) const;
  Vector operator*(const Vector &v) const;

  Mat4 Transposed() const;
  Mat4 Inverse() const;
};

// ============================================================================
// Mat4 Implementation
// ============================================================================

FORCEINLINE Mat4 Mat4::Identity()
{
  Mat4 result{};
  result.m[0] = 1.0f;
  result.m[5] = 1.0f;
  result.m[10] = 1.0f;
  result.m[15] = 1.0f;
  return result;
}

FORCEINLINE Mat4 Mat4::Translation(const Vector &position)
{
  Mat4 result = Identity();
  result.m[12] = position.x;
  result.m[13] = position.y;
  result.m[14] = position.z;
  return result;
}

FORCEINLINE Mat4 Mat4::Scale(const Vector &scale)
{
  Mat4 result{};
  result.m[0] = scale.x;
  result.m[5] = scale.y;
  result.m[10] = scale.z;
  result.m[15] = 1.0f;
  return result;
}

FORCEINLINE Mat4 Mat4::RotationX(float radians)
{
  Mat4 result = Identity();
  float c = cosf(radians);
  float s = sinf(radians);
  result.m[5] = c;
  result.m[6] = s;
  result.m[9] = -s;
  result.m[10] = c;
  return result;
}

FORCEINLINE Mat4 Mat4::RotationY(float radians)
{
  Mat4 result = Identity();
  float c = cosf(radians);
  float s = sinf(radians);
  result.m[0] = c;
  result.m[2] = -s;
  result.m[8] = s;
  result.m[10] = c;
  return result;
}

FORCEINLINE Mat4 Mat4::RotationZ(float radians)
{
  Mat4 result = Identity();
  float c = cosf(radians);
  float s = sinf(radians);
  result.m[0] = c;
  result.m[1] = s;
  result.m[4] = -s;
  result.m[5] = c;
  return result;
}

FORCEINLINE Mat4 Mat4::LookAt(const Vector &eye, const Vector &centre, const Vector &up)
{
  Vector back, right, trueUp;
  VectorSub(eye, centre, back);
  back.NormaliseInPlace();
  VectorCrossNorm(up, back, right);
  VectorCrossNorm(back, right, trueUp);

  Mat4 view = Identity();
  view.m[0] = right.x;
  view.m[1] = trueUp.x;
  view.m[2] = back.x;
  view.m[4] = right.y;
  view.m[5] = trueUp.y;
  view.m[6] = back.y;
  view.m[8] = right.z;
  view.m[9] = trueUp.z;
  view.m[10] = back.z;
  view.m[12] = -DotProduct(right, eye);
  view.m[13] = -DotProduct(trueUp, eye);
  view.m[14] = -DotProduct(back, eye);
  return view;
}

FORCEINLINE Mat4 Mat4::Perspective(float fov, float aspect, float znear, float zfar)
{
  Mat4 result{};
  float f = 1.0f / tanf(fov * 0.5f);
  result.m[0] = f / aspect;
  result.m[5] = f;
  result.m[10] = (zfar + znear) / (znear - zfar);
  result.m[11] = -1.0f;
  result.m[14] = (2.0f * zfar * znear) / (znear - zfar);
  return result;
}

FORCEINLINE Mat4 Mat4::Orthographic(float left, float right, float bottom, float top, float znear, float zfar)
{
  Mat4 result = Identity();
  result.m[0] = 2.0f / (right - left);
  result.m[5] = 2.0f / (top - bottom);
  result.m[10] = -2.0f / (zfar - znear);
  result.m[12] = -(right + left) / (right - left);
  result.m[13] = -(top + bottom) / (top - bottom);
  result.m[14] = -(zfar + znear) / (zfar - znear);
  return result;
}

FORCEINLINE Mat4 Mat4::operator*(const Mat4 &b) const
{
  Mat4 result{};
  for (int col = 0; col < 4; ++col)
  {
    for (int row = 0; row < 4; ++row)
    {
      float value = 0.0f;
      for (int k = 0; k < 4; ++k)
      {
        value += m[k * 4 + row] * b.m[col * 4 + k];
      }
      result.m[col * 4 + row] = value;
    }
  }
  return result;
}

FORCEINLINE Vector Mat4::operator*(const Vector &v) const
{
  Vector result;
  result.x = m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12];
  result.y = m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13];
  result.z = m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14];
  return result;
}

FORCEINLINE Mat4 Mat4::Transposed() const
{
  Mat4 result{};
  for (int row = 0; row < 4; ++row)
  {
    for (int col = 0; col < 4; ++col)
    {
      result.m[row * 4 + col] = m[col * 4 + row];
    }
  }
  return result;
}

FORCEINLINE Mat4 Mat4::Inverse() const
{
  Mat4 result{};
  const float *a = m;
  float *inv = result.m;

  inv[0] = a[5] * a[10] * a[15] - a[5] * a[11] * a[14] - a[9] * a[6] * a[15] +
           a[9] * a[7] * a[14] + a[13] * a[6] * a[11] - a[13] * a[7] * a[10];
  inv[4] = -a[4] * a[10] * a[15] + a[4] * a[11] * a[14] + a[8] * a[6] * a[15] -
           a[8] * a[7] * a[14] - a[12] * a[6] * a[11] + a[12] * a[7] * a[10];
  inv[8] = a[4] * a[9] * a[15] - a[4] * a[11] * a[13] - a[8] * a[5] * a[15] +
           a[8] * a[7] * a[13] + a[12] * a[5] * a[11] - a[12] * a[7] * a[9];
  inv[12] = -a[4] * a[9] * a[14] + a[4] * a[10] * a[13] + a[8] * a[5] * a[14] -
            a[8] * a[6] * a[13] - a[12] * a[5] * a[10] + a[12] * a[6] * a[9];
  inv[1] = -a[1] * a[10] * a[15] + a[1] * a[11] * a[14] + a[9] * a[2] * a[15] -
           a[9] * a[3] * a[14] - a[13] * a[2] * a[11] + a[13] * a[3] * a[10];
  inv[5] = a[0] * a[10] * a[15] - a[0] * a[11] * a[14] - a[8] * a[2] * a[15] +
           a[8] * a[3] * a[14] + a[12] * a[2] * a[11] - a[12] * a[3] * a[10];
  inv[9] = -a[0] * a[9] * a[15] + a[0] * a[11] * a[13] + a[8] * a[1] * a[15] -
           a[8] * a[3] * a[13] - a[12] * a[1] * a[11] + a[12] * a[3] * a[9];
  inv[13] = a[0] * a[9] * a[14] - a[0] * a[10] * a[13] - a[8] * a[1] * a[14] +
            a[8] * a[2] * a[13] + a[12] * a[1] * a[10] - a[12] * a[2] * a[9];
  inv[2] = a[1] * a[6] * a[15] - a[1] * a[7] * a[14] - a[5] * a[2] * a[15] +
           a[5] * a[3] * a[14] + a[13] * a[2] * a[7] - a[13] * a[3] * a[6];
  inv[6] = -a[0] * a[6] * a[15] + a[0] * a[7] * a[14] + a[4] * a[2] * a[15] -
           a[4] * a[3] * a[14] - a[12] * a[2] * a[7] + a[12] * a[3] * a[6];
  inv[10] = a[0] * a[5] * a[15] - a[0] * a[7] * a[13] - a[4] * a[1] * a[15] +
            a[4] * a[3] * a[13] + a[12] * a[1] * a[7] - a[12] * a[3] * a[5];
  inv[14] = -a[0] * a[5] * a[14] + a[0] * a[6] * a[13] + a[4] * a[1] * a[14] -
            a[4] * a[2] * a[13] - a[12] * a[1] * a[6] + a[12] * a[2] * a[5];
  inv[3] = -a[1] * a[6] * a[11] + a[1] * a[7] * a[10] + a[5] * a[2] * a[11] -
           a[5] * a[3] * a[10] - a[9] * a[2] * a[7] + a[9] * a[3] * a[6];
  inv[7] = a[0] * a[6] * a[11] - a[0] * a[7] * a[10] - a[4] * a[2] * a[11] +
           a[4] * a[3] * a[10] + a[8] * a[2] * a[7] - a[8] * a[3] * a[6];
  inv[11] = -a[0] * a[5] * a[11] + a[0] * a[7] * a[9] + a[4] * a[1] * a[11] -
            a[4] * a[3] * a[9] - a[8] * a[1] * a[7] + a[8] * a[3] * a[5];
  inv[15] = a[0] * a[5] * a[10] - a[0] * a[6] * a[9] - a[4] * a[1] * a[10] +
            a[4] * a[2] * a[9] + a[8] * a[1] * a[6] - a[8] * a[2] * a[5];

  float det = a[0] * inv[0] + a[1] * inv[4] + a[2] * inv[8] + a[3] * inv[12];
  if (det == 0.0f)
    return Identity();

  det = 1.0f / det;
  for (int i = 0; i < 16; ++i)
    inv[i] *= det;
  return result;
}

#endif // MATH_MATRIX_HPP

#endif
