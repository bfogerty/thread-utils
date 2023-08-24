//
// Created by Brandon Fogerty on 8/23/23.
//

#ifndef _RINGBUFFER_H
#define _RINGBUFFER_H

template <typename T>
class RingBuffer
{
public:
    RingBuffer(const size_t length) :
            m_arraySize(length)
            , m_readIndex(0)
            , m_writeIndex(0)
    {
        m_array = (T*)malloc(m_arraySize * sizeof(T));
    }

    ~RingBuffer()
    {
        free(m_array);
    }

    RingBuffer(const RingBuffer& other) = delete;
    RingBuffer operator = (const RingBuffer& other) = delete;


    const bool isEmpty() const
    {
        return m_readIndex == m_writeIndex;
    }

    const bool isFull() const
    {
        if(m_arraySize <= (m_writeIndex - m_readIndex))
            return true;

        return false;
    }

    bool enqueue(const T& value)
    {
        if(isFull())
            return false;

        m_array[m_writeIndex % m_arraySize] = value;
        ++m_writeIndex;

        return true;
    }

    bool dequeue(T& outValue)
    {
        if(isEmpty())
            return false;

        outValue = m_array[m_readIndex % m_arraySize];
        ++m_readIndex;

        return true;
    }

private:
    T* m_array;
    size_t m_arraySize;
    uint32_t  m_readIndex;
    uint32_t  m_writeIndex;
};

#endif // _RINGBUFFER_H
