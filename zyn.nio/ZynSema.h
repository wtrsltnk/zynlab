#ifndef ZYNSEMA_H
#define ZYNSEMA_H

#if defined __APPLE__ || defined WIN32

#include <condition_variable>
#include <mutex>

class ZynSema
{
public:
    ZynSema()
        : _count(0)
    {}

    ~ZynSema() = default;

    int init(
        int s,
        int v)
    {
        (void)s;

        _count = v;

        return 0;
    }

    int post()
    {
        std::lock_guard<std::mutex> guard(_mutex);

        if (++_count == 1)
        {
            _cond.notify_all();
        }

        return 0;
    }

    int wait()
    {
        std::unique_lock<std::mutex> lock(_mutex);

        while (_count < 1)
        {
            _cond.wait(lock);
        }

        --_count;

        return 0;
    }

    bool trywait()
    {
        if (!_mutex.try_lock())
        {
            return false;
        }

        if (_count < 1)
        {
            _mutex.unlock();

            return false;
        }

        --_count;

        _mutex.unlock();

        return true;
    }

    int getvalue() const
    {
        return _count;
    }

private:
    int _count;
    std::mutex _mutex;
    std::condition_variable _cond;
};

#else // POSIX sempahore

#include <semaphore.h>

class ZynSema
{
public:
    ZynSema()
    {}

    ~ZynSema()
    {
        sem_destroy(&_sema);
    }

    int init(
        int s,
        int v)
    {
        return sem_init(&_sema, s, v);
    }

    int post()
    {
        return sem_post(&_sema);
    }

    int wait()
    {
        return sem_wait(&_sema);
    }

    int trywait()
    {
        return sem_trywait(&_sema);
    }

    int getvalue()
    {
        int v = 0;

        sem_getvalue(&_sema, &v);

        return v;
    }

private:
    sem_t _sema;
};

#endif // POSIX semapore

#endif // ZYNSEMA_H
