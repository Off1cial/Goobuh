#ifndef MATH_VECTOR_H
#define MATH_VECTOR_H

#include "common/common.h"

typedef float vec_t;

typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];

static vec3_t VEC_ZERO = {0, 0, 0};

static vec3_t AXIS_X = {1, 0, 0};
static vec3_t AXIS_Y = {0, 1, 0};
static vec3_t AXIS_Z = {0, 0, 1};

static vec3_t AXIS_XN = {-1, 0, 0};
static vec3_t AXIS_YN = {0, -1, 0};
static vec3_t AXIS_ZN = {0, 0, -1};

#define PITCH 0
#define YAW 1
#define ROLL 2

typedef vec3_t qangle; // radians

FORCEINLINE void VectorAngles(const vec3_t v, qangle out){
  float pitch = atan2f(
      v[1],
      sqrtf(v[0] * v[0] + v[2] * v[2]));

  float yaw = atan2f(v[2], v[0]);

  out[PITCH] = pitch;
  out[YAW] = yaw;
  out[ROLL] = 0.0f;
}

FORCEINLINE void AnglesVector(const qangle angles, vec3_t out){
  float pitch = angles[PITCH];
  float yaw = angles[YAW];

  out[0] = cosf(yaw) * cosf(pitch);
  out[1] = sinf(pitch);
  out[2] = sinf(yaw) * cosf(pitch);
}


#define DotProduct(a, b) (a[0]*b[0]+a[1]*b[1]+a[2]*b[2])


#define VectorCopy(src, dst) dst[0]=src[0];dst[1]=src[1];dst[2]=src[2]


FORCEINLINE void VectorSet(vec3_t v, vec_t x, vec_t y, vec_t z)
{
    v[0] = x;
    v[1] = y;
    v[2] = z;
}

FORCEINLINE void VectorAdd(const vec3_t a, const vec3_t b, vec3_t out)
{
    out[0] = a[0] + b[0];
    out[1] = a[1] + b[1];
    out[2] = a[2] + b[2];
}

FORCEINLINE void VectorSub(const vec3_t a, const vec3_t b, vec3_t out)
{
    out[0] = a[0] - b[0];
    out[1] = a[1] - b[1];
    out[2] = a[2] - b[2];
}

FORCEINLINE void VectorScale(const vec3_t a, vec_t s, vec3_t out)
{
    out[0] = a[0] * s;
    out[1] = a[1] * s;
    out[2] = a[2] * s;
}

FORCEINLINE void VectorMA(const vec3_t a, vec_t s, const vec3_t b, vec3_t out)
{
    out[0] = a[0] + s * b[0];
    out[1] = a[1] + s * b[1];
    out[2] = a[2] + s * b[2];
}

FORCEINLINE vec_t VectorDot(
    const vec3_t a,
    const vec3_t b)
{
    return a[0] * b[0] +
           a[1] * b[1] +
           a[2] * b[2];
}

FORCEINLINE void VectorCross(
    const vec3_t a,
    const vec3_t b,
    vec3_t out)
{
    out[0] = a[1] * b[2] - a[2] * b[1];
    out[1] = a[2] * b[0] - a[0] * b[2];
    out[2] = a[0] * b[1] - a[1] * b[0];
}


FORCEINLINE vec_t VectorLength2(
    const vec3_t v)
{
    return VectorDot(v, v);
}

FORCEINLINE vec_t VectorLength(
    const vec3_t v)
{
    return sqrtf(VectorLength2(v));
}




FORCEINLINE vec_t VectorNormalise(
    vec3_t v)
{
    vec_t length = VectorLength(v);

    if (length == 0.0f)
        return 0.0f;

    vec_t inv_length = 1.0f / length;

    v[0] *= inv_length;
    v[1] *= inv_length;
    v[2] *= inv_length;

    return length;
}


FORCEINLINE void VectorCrossNorm(const vec3_t a, const vec3_t b, vec3_t out)
{
    VectorCross(a, b, out);
    VectorNormalise(out);
}


FORCEINLINE void VectorNormalised(
    const vec3_t v,
    vec3_t out)
{
    vec_t length = VectorLength(v);

    if (length == 0.0f)
    {
        out[0] = 0.0f;
        out[1] = 0.0f;
        out[2] = 0.0f;
        return;
    }

    vec_t inv_length = 1.0f / length;

    out[0] = v[0] * inv_length;
    out[1] = v[1] * inv_length;
    out[2] = v[2] * inv_length;
}

FORCEINLINE int VectorIsZero(
    const vec3_t v,
    vec_t tolerance)
{
    return fabsf(v[0]) < tolerance &&
           fabsf(v[1]) < tolerance &&
           fabsf(v[2]) < tolerance;
}

FORCEINLINE int VectorCompare(
    const vec3_t a,
    const vec3_t b)
{
    return a[0] == b[0] &&
           a[1] == b[1] &&
           a[2] == b[2];
}


#endif
