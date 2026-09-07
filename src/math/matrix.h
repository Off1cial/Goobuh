#ifndef MATRIX_H
#define MATRIX_H

#include "common/common.h"
#include "math/vector.h"

#include <math.h>
#include <string.h>

typedef float mat4[16];

static mat4 MATRIX_IDENTITY = {
  1, 0, 0, 0,
  0, 1, 0, 0,
  0, 0, 1, 0,
  0, 0, 0, 1
};


FORCEINLINE void MatrixIdentity(mat4 out)
{
  memset(out, 0, sizeof(mat4));

  out[0] = 1.0f;
  out[5] = 1.0f;
  out[10] = 1.0f;
  out[15] = 1.0f;
}

FORCEINLINE void MatrixTranslation(vec3_t position, mat4 out)
{
  MatrixIdentity(out);

  out[12] = position[0];
  out[13] = position[1];
  out[14] = position[2];
}

FORCEINLINE void MatrixScale(vec3_t scale, mat4 out)
{
  memset(out, 0, sizeof(mat4));

  out[0] = scale[0];
  out[5] = scale[1];
  out[10] = scale[2];
  out[15] = 1.0f;
}

FORCEINLINE void MatrixRotationX(float radians, mat4 out)
{
  MatrixIdentity(out);

  float c = cosf(radians);
  float s = sinf(radians);

  out[5] = c;
  out[6] = s;
  out[9] = -s;
  out[10] = c;
}

FORCEINLINE void MatrixRotationY(float radians, mat4 out)
{
  MatrixIdentity(out);

  float c = cosf(radians);
  float s = sinf(radians);

  out[0] = c;
  out[2] = -s;
  out[8] = s;
  out[10] = c;
}

FORCEINLINE void MatrixRotationZ(float radians, mat4 out)
{
  MatrixIdentity(out);

  float c = cosf(radians);
  float s = sinf(radians);

  out[0] = c;
  out[1] = s;
  out[4] = -s;
  out[5] = c;
}

FORCEINLINE void MatrixLookAt(vec3_t eye, vec3_t centre, vec3_t up, mat4 out)
{
  vec3_t back;
  vec3_t right;
  vec3_t true_up;

  VectorSub(eye, centre, back);
  VectorNormalise(back);
  VectorCrossNorm(up, back, right);
  VectorCrossNorm(back, right, true_up);

  MatrixIdentity(out);

  out[0] = right[0];
  out[1] = true_up[0];
  out[2] = back[0];

  out[4] = right[1];
  out[5] = true_up[1];
  out[6] = back[1];

  out[8] = right[2];
  out[9] = true_up[2];
  out[10] = back[2];

  out[12] = -VectorDot(right, eye);
  out[13] = -VectorDot(true_up, eye);
  out[14] = -VectorDot(back, eye);
}

FORCEINLINE void MatrixPerspective(float fov, float aspect, float znear, float zfar, mat4 out)
{
  memset(out, 0, sizeof(mat4));

  float f = 1.0f / tanf(fov * 0.5f);

  out[0] = f / aspect;
  out[5] = f;
  out[10] = (zfar + znear) / (znear - zfar);
  out[11] = -1.0f;
  out[14] = (2.0f * zfar * znear) / (znear - zfar);
}

FORCEINLINE void MatrixOrthographic(float left, float right, float bottom, float top, float znear, float zfar, mat4 out)
{
  MatrixIdentity(out);

  out[0] = 2.0f / (right - left);
  out[5] = 2.0f / (top - bottom);
  out[10] = -2.0f / (zfar - znear);
  out[12] = -(right + left) / (right - left);
  out[13] = -(top + bottom) / (top - bottom);
  out[14] = -(zfar + znear) / (zfar - znear);
}

FORCEINLINE void MatrixMultiply(mat4 a, mat4 b, mat4 out)
{
  mat4 result;

  for (int col = 0; col < 4; ++col)
  {
    for (int row = 0; row < 4; ++row)
    {
      result[col * 4 + row] = 0.0f;

      for (int k = 0; k < 4; ++k)
        result[col * 4 + row] += a[k * 4 + row] * b[col * 4 + k];
    }
  }

  memcpy(out, result, sizeof(mat4));
}

FORCEINLINE void MatrixTransform(vec3_t v, mat4 matrix, vec3_t out)
{
  out[0] = matrix[0] * v[0] + matrix[4] * v[1] + matrix[8] * v[2] + matrix[12];
  out[1] = matrix[1] * v[0] + matrix[5] * v[1] + matrix[9] * v[2] + matrix[13];
  out[2] = matrix[2] * v[0] + matrix[6] * v[1] + matrix[10] * v[2] + matrix[14];
}

FORCEINLINE void MatrixTranspose(mat4 matrix, mat4 out)
{
  mat4 result;

  for (int row = 0; row < 4; ++row)
  {
    for (int col = 0; col < 4; ++col)
      result[row * 4 + col] = matrix[col * 4 + row];
  }

  memcpy(out, result, sizeof(mat4));
}

FORCEINLINE void MatrixInverse(mat4 matrix, mat4 out)
{
  mat4 result;
  const float *a = matrix;
  float *inv = result;

  inv[0] = a[5] * a[10] * a[15] - a[5] * a[11] * a[14] - a[9] * a[6] * a[15] + a[9] * a[7] * a[14] + a[13] * a[6] * a[11] - a[13] * a[7] * a[10];
  inv[4] = -a[4] * a[10] * a[15] + a[4] * a[11] * a[14] + a[8] * a[6] * a[15] - a[8] * a[7] * a[14] - a[12] * a[6] * a[11] + a[12] * a[7] * a[10];
  inv[8] = a[4] * a[9] * a[15] - a[4] * a[11] * a[13] - a[8] * a[5] * a[15] + a[8] * a[7] * a[13] + a[12] * a[5] * a[11] - a[12] * a[7] * a[9];
  inv[12] = -a[4] * a[9] * a[14] + a[4] * a[10] * a[13] + a[8] * a[5] * a[14] - a[8] * a[6] * a[13] - a[12] * a[5] * a[10] + a[12] * a[6] * a[9];

  inv[1] = -a[1] * a[10] * a[15] + a[1] * a[11] * a[14] + a[9] * a[2] * a[15] - a[9] * a[3] * a[14] - a[13] * a[2] * a[11] + a[13] * a[3] * a[10];
  inv[5] = a[0] * a[10] * a[15] - a[0] * a[11] * a[14] - a[8] * a[2] * a[15] + a[8] * a[3] * a[14] + a[12] * a[2] * a[11] - a[12] * a[3] * a[10];
  inv[9] = -a[0] * a[9] * a[15] + a[0] * a[11] * a[13] + a[8] * a[1] * a[15] - a[8] * a[3] * a[13] - a[12] * a[1] * a[11] + a[12] * a[3] * a[9];
  inv[13] = a[0] * a[9] * a[14] - a[0] * a[10] * a[13] - a[8] * a[1] * a[14] + a[8] * a[2] * a[13] + a[12] * a[1] * a[10] - a[12] * a[2] * a[9];

  inv[2] = a[1] * a[6] * a[15] - a[1] * a[7] * a[14] - a[5] * a[2] * a[15] + a[5] * a[3] * a[14] + a[13] * a[2] * a[7] - a[13] * a[3] * a[6];
  inv[6] = -a[0] * a[6] * a[15] + a[0] * a[7] * a[14] + a[4] * a[2] * a[15] - a[4] * a[3] * a[14] - a[12] * a[2] * a[7] + a[12] * a[3] * a[6];
  inv[10] = a[0] * a[5] * a[15] - a[0] * a[7] * a[13] - a[4] * a[1] * a[15] + a[4] * a[3] * a[13] + a[12] * a[1] * a[7] - a[12] * a[3] * a[5];
  inv[14] = -a[0] * a[5] * a[14] + a[0] * a[6] * a[13] + a[4] * a[1] * a[14] - a[4] * a[2] * a[13] - a[12] * a[1] * a[6] + a[12] * a[2] * a[5];

  inv[3] = -a[1] * a[6] * a[11] + a[1] * a[7] * a[10] + a[5] * a[2] * a[11] - a[5] * a[3] * a[10] - a[9] * a[2] * a[7] + a[9] * a[3] * a[6];
  inv[7] = a[0] * a[6] * a[11] - a[0] * a[7] * a[10] - a[4] * a[2] * a[11] + a[4] * a[3] * a[10] + a[8] * a[2] * a[7] - a[8] * a[3] * a[6];
  inv[11] = -a[0] * a[5] * a[11] + a[0] * a[7] * a[9] + a[4] * a[1] * a[11] - a[4] * a[3] * a[9] - a[8] * a[1] * a[7] + a[8] * a[3] * a[5];
  inv[15] = a[0] * a[5] * a[10] - a[0] * a[6] * a[9] - a[4] * a[1] * a[10] + a[4] * a[2] * a[9] + a[12] * a[1] * a[6] - a[12] * a[2] * a[5];

  float det = a[0] * inv[0] + a[1] * inv[4] + a[2] * inv[8] + a[3] * inv[12];

  if (det == 0.0f)
  {
    MatrixIdentity(out);
    return;
  }

  float inv_det = 1.0f / det;

  for (int i = 0; i < 16; ++i)
    inv[i] *= inv_det;

  memcpy(out, result, sizeof(mat4));
}

#endif
