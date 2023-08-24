#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <thread>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <vector>
#include "ThreadPoolTask.h"

// Derived from the implementation designed by the Coding Blacksmith
// https://www.youtube.com/watch?v=eWTGtp3HXiw&t=607s
class ThreadPool
{
public:
    ThreadPool(const size_t maxThreadCount) :
            m_maxThreadCount(maxThreadCount)
            , m_stopping(false)
            , m_scheduledTasksCounter(0)
            , m_taskCounter(0)
    {
        start();
    }

    ~ThreadPool()
    {
        stop();
    }

    ThreadPool(const ThreadPool& other) = delete;
    ThreadPool& operator = (const ThreadPool& other) = delete;

    const ThreadPoolTask& enqueue(void* data, void (*func)(const uint64_t taskId, void* data));
    void flush();

private:
    void start();
    void stop();
    const bool empty() const;

    size_t m_maxThreadCount;
    bool m_stopping;
    std::mutex m_mutex;
    std::condition_variable m_threadEventVar;
    std::vector<std::thread> m_threads;
    std::queue<ThreadPoolTask> m_TaskQueue;
    std::atomic<uint32_t> m_scheduledTasksCounter;
    std::atomic<uint64_t> m_taskCounter;
};

#endif //_THREADPOOL_H
