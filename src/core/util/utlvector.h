#ifndef UTLVECTOR_H
#define UTLVECTOR_H


#include "core/util/utlmemory.h"

// Genuinely annoying to type
#define TEMP_TM template <typename T, typename M>

template < typename T, class M = CUtlMemory<T> >
class CUtlVector
{     
    typedef M Memory;
    typedef T* iterator;
    typedef const T* const_iterator;

public:
    explicit CUtlVector( int initSize, float growthFactor );
    explicit CUtlVector( T* pData, int initSize, float growthFactor );
    CUtlVector();
    ~CUtlVector();

    void Purge();

    // Valid index for accessing
    bool IsValidIndex( int index ) const { return (uint32)index < m_iCount; }
    int AddToHead( const T &src );
    int AddToTail( const T &src );


    int InsertBefore( int index, const T &src );
    int InsertAfter( int index, const T &src );
    
    T& Element( int index ) { return m_memory[index]; }
    const T& Element( int index ) const { return m_memory[index]; }

    const T& operator[]( uint32 i ) const;
    T& operator[]( uint32 i );

    // Requires that 'T' has an == operator
    int Find( const T &src ) const;

    void EnsureCapacity( int capacity );

    int Count() const { return m_iCount; }
    T* Base() const { return m_memory.Base(); }

    // Grow to fit this many MORE elements;
    void GrowVector( int num = 1 );


private:
    void Construct( T* dst, const T src );
    void ShiftElementsRight( int fromInclusive, int numPlaces = 1 );
    void ShiftElementsLeft( int fromInclusive, int numPlaces = 1 );

private:
    uint32 m_iCount = 0;
    Memory m_memory;

};

template <typename T, typename M> 
CUtlVector<T, M>::CUtlVector( int initSize, float growthFactor )
    : m_memory( initSize, growthFactor )
{
    Assert( initSize >= 0 );
    Assert( growthFactor >= 0.0f );
}

template <typename T, typename M> 
CUtlVector<T, M>::CUtlVector( T* pData, int initSize, float growthFactor )
    : m_memory( pData, initSize, growthFactor )
{
    Assert( initSize >= 0 );
    Assert( growthFactor >= 0.0f );
    m_iCount = initSize;
}

template <typename T, typename M>
CUtlVector<T, M>::~CUtlVector()
{
    Purge();
}

template <typename T, typename M>
inline const T& CUtlVector<T, M>::operator[](uint32 i) const
{
    Assert(i < m_iCount);
    return m_memory[i];
}

template <typename T, typename M>
inline T& CUtlVector<T, M>::operator[](uint32 i)
{
    Assert(i < m_iCount);
    return m_memory[i];
}

// Grow to fit this many elements
template <typename T, typename M>
void CUtlVector<T, M>::GrowVector( int num )
{
    Assert( num > 0 );
    if ( m_iCount + num > m_memory.Size())
    {
        m_memory.Grow( m_iCount + num - m_memory.Size() );
    }
}


template <typename T, typename M>
void CUtlVector<T, M>::ShiftElementsRight( int fromInclusive, int numPlaces )
{
    Assert( numPlaces > 0 );
    int numToMove = m_iCount - fromInclusive;
    if (numToMove > 0)
    {
        T *base = Base();
        memmove( 
            base + fromInclusive + numPlaces,
            base + fromInclusive,
            sizeof(T) * numToMove
          );
    }
}

template <typename T, typename M>
void CUtlVector<T, M>::ShiftElementsLeft( int fromInclusive, int numPlaces )
{
    // FIXME: What if fromInclusive - numplaces < 0
    Assert( IsValidIndex( fromInclusive ) );
    Assert( numPlaces > 0 );
    Assert( fromInclusive - numPlaces >= 0 );

    int numToMove = m_iCount - fromInclusive;
    if ( numToMove > 0 )
    {
        T* base = Base();
        memmove( base + fromInclusive - numPlaces, base + fromInclusive, numToMove * sizeof(T));
    }
}

template <typename T, typename M>
inline int CUtlVector<T, M>::InsertBefore( int index, const T& src )
{
    Assert( index == m_iCount || IsValidIndex( index ) );
    // Ensure the element we are inserting isnt a reference to an existing element
    Assert( (Base() == nullptr) || (&src < Base()) || (&src >= Base() + Count()));

    GrowVector(); 
    ShiftElementsRight( index );
    CopyConstruct( &m_memory[index], src );
    ++m_iCount;
    return index;
}

template <typename T, typename M>
inline int CUtlVector<T, M>::InsertAfter( int index, const T& src )
{
    return InsertBefore( index + 1, src );
}


template <typename T, typename M>
inline int CUtlVector<T, M>::AddToHead( const T& src )
{
    return InsertBefore(0, src);
}

template <typename T, typename M>
inline int CUtlVector<T, M>::AddToTail( const T& src )
{
    return InsertBefore( m_iCount , src);
}


template <typename T, typename M>
inline void CUtlVector<T, M>::Purge()
{
    for (int i = m_iCount - 1; i >= 0; i--)
    {
        m_memory[i].~T();
    }
}



#undef TEMP_TM

#endif
