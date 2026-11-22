#ifndef UTLLINKEDLIST_H
#define UTLLINKEDLIST_H

#include "core/util/utlmemory.h"

template <typename T, typename I>
struct LinkedListElement
{
    T data;
    I next;
    I prev;
};

template <typename T, typename M = CUtlMemory<T>>
class CUtlLinkedList
{   
public:
// Index accessibility on a linked list!?!?!?
    T operator[]( uint32 i ) const;
    T& operator[]( uint32 i );

private:
};

#endif
