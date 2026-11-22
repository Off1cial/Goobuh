#ifndef __ALLOCATOR_H__
#define __ALLOCATOR_H__

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <type_traits>
#include <utility>
#include "core/defs.h"
#include "core/assert.h"

#define INVALID_ALLOCATION 0xDEADBEEF
#define GROWTH_MAX 3.0F
#define GROWTH_MIN 1.25F



template <typename T>
class CUtlMemory
{
public:
    explicit CUtlMemory( uint32 initSize = 0, float growFactor = 0.0f );
    // Initialise with a pre-defined set of data,
    // Copies the data, does not share the allocation
    explicit CUtlMemory( T* pData, uint32 initSize, float growFactor );
    ~CUtlMemory();
    
    T* Base() const { return m_pData; };
    int32 Size() const;
    float GrowthFactor() const;
    // Elements should be strictly accessed by index, 
    // never pointer as realloc will invalid pointers

    // Growth that automatically uses the growth factor
    void Grow(); 
    // Growth that takes in a set number of elements to grow by
    void Grow( int num );
    void EnsureCapacity( int capacity );
    // Array-style access
    T operator[]( int32 i ) const;
    T& operator[] (int32 i );

private:
    
protected:
    T* m_pData = nullptr;
    uint32 m_iSize = 0; 
    float m_fGrowth = 0.0f;

};

template <typename T>
CUtlMemory<T>::CUtlMemory( uint32 initSize, float growFactor )
{
    Assert( initSize < SIZE_MAX / sizeof(T) );    
    Assert( growFactor >= 0.0f );

    m_iSize = initSize;
    m_fGrowth = MIN( GROWTH_MAX, MAX( GROWTH_MIN, growFactor ) );

    if (m_iSize > 0 )
    {
        m_pData = static_cast<T*>( calloc(1, sizeof(T) * m_iSize) );
        Assert( m_pData );
    }
}


template <class T>
inline T* Construct( T* pMemory )
{
    return ::new( pMemory ) T;
}

template <class T>
inline T* CopyConstruct( T* pMemory, T const& src )
{
    return ::new( pMemory ) T( src );
}

template <class T>
inline void Destruct( T* pMemory )
{
    pMemory->~T();

#ifdef _DEBUG
    memset( pMemory, 0xDD, sizeof(T) );   // poison the bytes so use-after-destroy is obvious
#endif
}

template <typename T>
CUtlMemory<T>::CUtlMemory( T* pData, uint32 initSize, float growFactor )
{
    Assert( initSize < SIZE_MAX / sizeof(T) && initSize > 0 );    
    Assert( growFactor >= 0.0f );

    m_iSize = initSize;
    m_fGrowth = MIN( GROWTH_MAX, MAX( GROWTH_MIN, growFactor ) );
        
    m_pData = static_cast<T*>( calloc(1, sizeof(T) * m_iSize) );
    Assert( m_pData );

    memcpy( m_pData, pData, sizeof(T) * initSize );
}

template <typename T>
void CUtlMemory<T>::Grow()
{
    Assert( m_fGrowth > GROWTH_MIN );

    /*
    size_t bytes = sizeof(T) * m_iSize * m_fGrowth;
    T* alloc = static_cast<T*>( realloc( m_pData, bytes ) );
    Assert( alloc );
        
    size_t difference = (m_iSize * m_fGrowth) - m_iSize;
    memset( alloc + m_iSize, 0, sizeof(T) * difference );

#ifdef _DEBUG
printf("CUtlMemory: Growth from %dB to %dB\n", m_iSize * sizeof(T), bytes);
#endif
    m_iSize = m_iSize * m_fGrowth;
    m_pData = alloc;
    */
    Grow( m_iSize * ( m_fGrowth - 1));
}


template <typename T> 
void CUtlMemory<T>::Grow( int num )
{
    Assert( num > 0 );

    int byPolicy = (int)( m_iSize * ( m_fGrowth - 1 ) );
    if ( num < byPolicy ) num = byPolicy;  

    T* alloc = static_cast<T*>( realloc( m_pData, sizeof(T) * (num + m_iSize) ));
    Assert( alloc );

    //memset( alloc + m_iSize, 0, num * sizeof(T) );

    m_pData = alloc;
    m_iSize += num;
#ifdef _DEBUG
printf("CUtlMemory: Growth from %dB to %dB\n", m_iSize * sizeof(T), sizeof(T) * (m_iSize + num));
#endif
}


template <typename T>
CUtlMemory<T>::~CUtlMemory()
{
    free(m_pData);
}

template <typename T>
int32 CUtlMemory<T>::Size() const
{
    return m_iSize;
}

template <typename T>
float CUtlMemory<T>::GrowthFactor() const
{
    return m_fGrowth;
}

template <typename T>
T CUtlMemory<T>::operator[](int32 i) const
{
    Assert(i >= 0 && i < m_iSize);
    return m_pData[i];
}

template <typename T>
T& CUtlMemory<T>::operator[](int32 i)
{
    Assert(i >= 0 && i < m_iSize);
    return m_pData[i];
}

#endif
