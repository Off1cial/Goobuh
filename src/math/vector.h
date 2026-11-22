#ifndef __VECTOR_H__
#define __VECTOR_H__

#include "core/defs.h"

#define Vec2String( v ) "(%0.2f, %0.2f, %0.2f)\n", (v).x, (v).y, (v).z

class Vector
{
public:
    float x,y,z;
    
    Vector() { x = 0; y = 0; z = 0;}
    Vector( float _x, float _y, float _z) { x = _x, y = _y, z = _z;}
    // Array access
    float  operator[] (int i) const;
    float &operator[] (int i); 

    FORCEINLINE Vector& operator+= (const Vector &v);
    FORCEINLINE Vector& operator-= (const Vector &v);

    FORCEINLINE Vector& operator*= (const float f);
    FORCEINLINE Vector& operator+= (const float f);
    FORCEINLINE Vector& operator-= (const float f);

    FORCEINLINE float LengthSqr();
    FORCEINLINE float Length();

    FORCEINLINE float NormaliseInPlace();
    FORCEINLINE Vector Normalised();

};

float Vector::LengthSqr()
{
    return x * x + y * y + z * z;
}

float Vector::Length()
{
    return sqrtf(x*x + y*y + z*z);
}

float Vector::NormaliseInPlace()
{
    float len = Length();
    float recip = 1.0f / len;
    x *= recip;
    y *= recip;
    z *= recip;
    return len;
}

Vector Vector::Normalised()
{
    Vector v(x,y,z);
    v.NormaliseInPlace();
    return v;
}


Vector& Vector::operator+= (const Vector &v)
{
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
}

Vector& Vector::operator-= (const Vector &v)
{
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
}

Vector& Vector::operator+= (const float f)
{
    x += f;
    y += f;
    z += f;
    return *this;
}

Vector& Vector::operator-= (const float f)
{
    x -= f;
    y -= f;
    z -= f;
    return *this;
}

Vector& Vector::operator*= (const float f)
{
    x *= f;
    y *= f;
    z *= f;
    return *this;
}



inline float Vector::operator[](int i) const
{
    return ((float*)this)[i];
}
inline float& Vector::operator[](int i)
{
    return ((float*)this)[i];
}

#endif
