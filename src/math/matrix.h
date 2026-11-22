#ifndef __MATRIX_H__
#define __MATRIX_H__

#include "core/defs.h"
#include "math/vector.h"

#define MatrixMN Matrix<M,N>

#define MNTEMP template<int M, int N>

template<int M, int N>
class Matrix
{
public:
    Matrix() {}

    Matrix<M,M> operator* (const Matrix<N,M> &b) const;

public:
    float m[M][N];
};


template<int M, int N>
inline Matrix<M,M> Matrix<M,N>::operator*(const Matrix<N,M>& b) const
{
    Matrix<M,M> ret;

    for (int row = 0; row < M; row++)
    {
        for (int col = 0; col < M; col++)
        {
            ret.m[row][col] = 0.0f;

            for (int k = 0; k < N; ++k)
                ret.m[row][col] += m[row][k] * b.m[k][col];
        }
    }

    return ret;
}


// Specialised
class Matrix4
{
public:
    Matrix4() {}

    static void Zero();
    static void Identity();
    
    Vector operator* (const Vector& v) const;
    Matrix4 &operator* (const Matrix4& other);


private:
    float m[16];
};


#endif
