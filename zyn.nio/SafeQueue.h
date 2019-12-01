
#ifndef SAFEQUEUE_H
#define SAFEQUEUE_H

#include "ZynSema.h"
#include <cstdlib>
#include <pthread.h>

/**
 * C++ thread safe lockless queue
 * Based off of jack's ringbuffer*/
template <class T>
class SafeQueue
{
public:
    SafeQueue(size_t maxlen)
        : writePtr(0), readPtr(0), bufSize(maxlen)
    {
        w_space.init(PTHREAD_PROCESS_PRIVATE, maxlen - 1);
        r_space.init(PTHREAD_PROCESS_PRIVATE, 0);
        buffer = new T[maxlen];
    }

    ~SafeQueue() { delete[] buffer; }

    unsigned int size() const { return rSpace(); }

    int push(const T &in)
    {
        if (!wSpace())
        {
            return -1;
        }

        //ok, there is space to write
        size_t w = (writePtr + 1) % bufSize;
        buffer[w] = in;
        writePtr = w;

        //adjust ranges
        w_space.wait(); //guaranteed not to wait
        r_space.post();
        return 0;
    }

    int peak(T &out) const
    {
        if (!rSpace())
        {
            return -1;
        }

        //ok, there is space to read
        size_t r = (readPtr + 1) % bufSize;
        out = buffer[r];

        return 0;
    }

    int pop(T &out)
    {
        if (!rSpace())
        {
            return -1;
        }

        //ok, there is space to read
        size_t r = (readPtr + 1) % bufSize;
        out = buffer[r];
        readPtr = r;

        //adjust ranges
        r_space.wait(); //guaranteed not to wait
        w_space.post();
        return 0;
    }

    void clear()
    {
        //thread unsafe
        while (!r_space.trywait())
        {
            w_space.post();
        }
        readPtr = writePtr;
    }

private:
    unsigned int wSpace() const { return w_space.getvalue(); }
    unsigned int rSpace() const { return r_space.getvalue(); }

    //write space
    mutable ZynSema w_space;
    //read space
    mutable ZynSema r_space;

    //next writing spot
    size_t writePtr;
    //next reading spot
    size_t readPtr;
    const size_t bufSize;
    T *buffer;
};

#endif
