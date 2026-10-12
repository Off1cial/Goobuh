#ifndef MATHQUAT_H
#define MATHQUAT_H

#include "common/common.h"
#include "math/vector.h"
#include "math/matrix.h"

#include <math.h>

#define QuatCopy( src, dst ) dst[0]=src[0];dst[1]=src[1];dst[2]=src[2];dst[3]=src[3]

#ifdef __cplusplus
extern "C" {
#endif

// quat_t is a vec4_t stored as x, y, z, w (indices 0..3).

FORCEINLINE void QuatIdentity( quat_t q )
{
    q[0] = 0.0f;
    q[1] = 0.0f;
    q[2] = 0.0f;
    q[3] = 1.0f;
}

// out = a * b (apply b first, then a). Safe if out aliases a or b.
FORCEINLINE void QuatMultiply( const quat_t a, const quat_t b, quat_t out )
{
    quat_t r;

    r[0] = a[3] * b[0] + a[0] * b[3] + a[1] * b[2] - a[2] * b[1];
    r[1] = a[3] * b[1] - a[0] * b[2] + a[1] * b[3] + a[2] * b[0];
    r[2] = a[3] * b[2] + a[0] * b[1] - a[1] * b[0] + a[2] * b[3];
    r[3] = a[3] * b[3] - a[0] * b[0] - a[1] * b[1] - a[2] * b[2];

    out[0] = r[0];
    out[1] = r[1];
    out[2] = r[2];
    out[3] = r[3];
}

FORCEINLINE void QuatNormalise( quat_t q )
{
    float len = sqrtf( q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3] );

    if (len < 1e-8f)
    {
        QuatIdentity( q );
        return;
    }

    float inv = 1.0f / len;
    q[0] *= inv;
    q[1] *= inv;
    q[2] *= inv;
    q[3] *= inv;
}

// q += 0.5 * dt * (w, 0) * q, then renormalise. w is world-space angular velocity.
FORCEINLINE void QuatIntegrate( quat_t q, const vec3_t w, float dt )
{
    quat_t wq = { w[0], w[1], w[2], 0.0f };
    quat_t dq;

    QuatMultiply( wq, q, dq );

    q[0] += 0.5f * dt * dq[0];
    q[1] += 0.5f * dt * dq[1];
    q[2] += 0.5f * dt * dq[2];
    q[3] += 0.5f * dt * dq[3];

    QuatNormalise( q );
}



FORCEINLINE void QuatFromDirection( const vec3_t direction, quat_t out )
{
    vec3_t from = { 0.0f, 1.0f, 0.0f };
    vec3_t to;

    VectorCopy( direction, to );
    VectorNormalise( to );

    float dot = VectorDot( from, to );

    // Same direction.
    if (dot > 0.999999f)
    {
        QuatIdentity( out );
        return;
    }

    // Opposite direction.
    if (dot < -0.999999f)
    {
        // 180 degrees around X.
        out[0] = 1.0f;
        out[1] = 0.0f;
        out[2] = 0.0f;
        out[3] = 0.0f;
        return;
    }

    vec3_t axis;
    VectorCross( from, to, axis );
    VectorNormalise( axis );

    float angle = acosf( dot );
    float half = angle * 0.5f;
    float s = sinf( half );

    out[0] = axis[0] * s;
    out[1] = axis[1] * s;
    out[2] = axis[2] * s;
    out[3] = cosf( half );
}

FORCEINLINE void QuatFromAngles( const qangle angles, quat_t out )
{
    float pitch = angles[PITCH];
    float yaw = angles[YAW];
    float roll = angles[ROLL];

    float sp = sinf( pitch * 0.5f );
    float cp = cosf( pitch * 0.5f );

    float sy = sinf( yaw * 0.5f );
    float cy = cosf( yaw * 0.5f );

    float sr = sinf( roll * 0.5f );
    float cr = cosf( roll * 0.5f );

    // R = Ry(yaw) * Rx(-pitch) * Rz(roll)
    out[0] = -sp * cy * cr + cp * sy * sr;
    out[1] = cp * sy * cr + sp * cy * sr;
    out[2] = cp * cy * sr + sp * sy * cr;
    out[3] = cp * cy * cr - sp * sy * sr;
}

#ifdef __cplusplus
}
#endif

#endif