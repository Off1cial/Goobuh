#ifndef MATRIX_H
#define MATRIX_H

#include "common/common.h"
#include "math/vector.h"

typedef struct mat4 {
float m[16];
} mat4;




#include "math/matrix.h"
#include <math.h>
#include <string.h>

FORCEINLINE void MatrixIdentity(mat4 out)
{
    memset(out.m, 0, sizeof(out.m));
    out.m[0] = 1.0f;
    out.m[5] = 1.0f;
    out.m[10] = 1.0f;
    out.m[15] = 1.0f;
}

FORCEINLINE void MatrixTranslation(vec3_t position, mat4 out)
{
    MatrixIdentity(out);
    out.m[12] = position[0];
    out.m[13] = position[1];
    out.m[14] = position[2];
}

FORCEINLINE void MatrixScale(vec3_t scale, mat4 out)
{
    memset(out.m, 0, sizeof(out.m));
    out.m[0] = scale[0];
    out.m[5] = scale[1];
    out.m[10] = scale[2];
    out.m[15] = 1.0f;
}

FORCEINLINE void MatrixRotationX(float radians, mat4 out)
{
    MatrixIdentity(out);

    float c = cosf(radians);
    float s = sinf(radians);

    out.m[5] = c;
    out.m[6] = s;
    out.m[9] = -s;
    out.m[10] = c;
}

FORCEINLINE void MatrixRotationY(float radians, mat4 out)
{
    MatrixIdentity(out);

    float c = cosf(radians);
    float s = sinf(radians);

    out.m[0] = c;
    out.m[2] = -s;
    out.m[8] = s;
    out.m[10] = c;
}

FORCEINLINE void MatrixRotationZ(float radians, mat4 out)
{
    MatrixIdentity(out);

    float c = cosf(radians);
    float s = sinf(radians);

    out.m[0] = c;
    out.m[1] = s;
    out.m[4] = -s;
    out.m[5] = c;
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

    out.m[0] = right[0];
    out.m[1] = true_up[0];
    out.m[2] = back[0];

    out.m[4] = right[1];
    out.m[5] = true_up[1];
    out.m[6] = back[1];

    out.m[8] = right[2];
    out.m[9] = true_up[2];
    out.m[10] = back[2];

    out.m[12] = -VectorDot(right, eye);
    out.m[13] = -VectorDot(true_up, eye);
    out.m[14] = -VectorDot(back, eye);
}

FORCEINLINE void MatrixPerspective(float fov, float aspect, float znear, float zfar, mat4 out)
{
    memset(out.m, 0, sizeof(out.m));

    float f = 1.0f / tanf(fov * 0.5f);

    out.m[0] = f / aspect;
    out.m[5] = f;
    out.m[10] = (zfar + znear) / (znear - zfar);
    out.m[11] = -1.0f;
    out.m[14] = (2.0f * zfar * znear) / (znear - zfar);
}

FORCEINLINE void MatrixOrthographic(float left, float right, float bottom, float top, float znear, float zfar, mat4 out)
{
    MatrixIdentity(out);

    out.m[0] = 2.0f / (right - left);
    out.m[5] = 2.0f / (top - bottom);
    out.m[10] = -2.0f / (zfar - znear);
    out.m[12] = -(right + left) / (right - left);
    out.m[13] = -(top + bottom) / (top - bottom);
    out.m[14] = -(zfar + znear) / (zfar - znear);
}

FORCEINLINE void MatrixMultiply(mat4 a, mat4 b, mat4 out)
{
    mat4 result;

    for (int col = 0; col < 4; ++col)
    {
        for (int row = 0; row < 4; ++row)
        {
            result.m[col * 4 + row] = 0.0f;

            for (int k = 0; k < 4; ++k)
                result.m[col * 4 + row] += a.m[k * 4 + row] * b.m[col * 4 + k];
        }
    }

    memcpy(out.m, result.m, sizeof(result.m));
}

FORCEINLINE void MatrixTransform(vec3_t v, mat4 matrix, vec3_t out)
{
    out[0] = matrix.m[0] * v[0] + matrix.m[4] * v[1] + matrix.m[8] * v[2] + matrix.m[12];
    out[1] = matrix.m[1] * v[0] + matrix.m[5] * v[1] + matrix.m[9] * v[2] + matrix.m[13];
    out[2] = matrix.m[2] * v[0] + matrix.m[6] * v[1] + matrix.m[10] * v[2] + matrix.m[14];
}

FORCEINLINE void MatrixTranspose(mat4 matrix, mat4 out)
{
    mat4 result;

    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 4; ++col)
            result.m[row * 4 + col] = matrix.m[col * 4 + row];
    }

    memcpy(out.m, result.m, sizeof(result.m));
}

FORCEINLINE void MatrixInverse(mat4 matrix, mat4 out)
{
    mat4 result;
    const float *a = matrix.m;
    float *inv = result.m;

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
    inv[15] = a[0] * a[5] * a[10] - a[0] * a[6] * a[9] - a[4] * a[1] * a[10] + a[4] * a[2] * a[9] + a[8] * a[1] * a[6] - a[8] * a[2] * a[5];

    float det = a[0] * inv[0] + a[1] * inv[4] + a[2] * inv[8] + a[3] * inv[12];

    if (det == 0.0f)
    {
        MatrixIdentity(out);
        return;
    }

    float inv_det = 1.0f / det;

    for (int i = 0; i < 16; ++i)
        inv[i] *= inv_det;

    memcpy(out.m, result.m, sizeof(result.m));
}


#endif
