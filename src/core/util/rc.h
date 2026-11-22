#ifndef __RC_H__
#define __RC_H__

#include "core/defs.h"
#include <atomic>
#include <cstdint>

// A safer pointer that modifies its element to record the number of pointers that describe this object, leading to no use after frees etc

class RcObject
{
public:
    FORCEINLINE uint32_t incRef()
    {
        return ++m_refcount;
    }
    FORCEINLINE uint32_t decRef()
    {
        return --m_refcount;
    }

    uint32_t Refs() { return m_refcount;}

private:
    // In case I wanted to do multi threading
    std::atomic<uint32_t> m_refcount = 0;
};

template <typename T> 
class Rc
{
    template <typename Tx>
    friend class Rc;
public:
    Rc() {}
    Rc(std::nullptr_t){}

    ~Rc() { decRef(); }

    Rc(T* object) : m_object(object)
    {this->incRef();}

    Rc(const Rc &other) : m_object(other.m_object)
    { this->incRef(); }
   
    Rc(Rc&& other) :m_object(other.m_object)
    {
        other.m_object = nullptr;
    }   
    
    Rc& operator = ( const Rc &other)
    {
        other.incRef();
        this->decRef();
        m_object = other.m_object;
        return *this;
    }


private:

    void incRef() const
    {
        if (nullptr != m_object)
            m_object->incRef();
    }

    void decRef() const
    {
        if (m_object != nullptr)
            if (m_object->decRef() == 0)
                delete m_object;
    }

    T* m_object;
};

#endif
