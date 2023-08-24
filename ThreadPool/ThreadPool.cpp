#include "ThreadPool.h"

const ThreadPoolTask& ThreadPool::enqueue(void* data, void (*func)(const uint64_t taskId, void* data))
{
    ThreadPoolTask* task = nullptr;
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        ++m_scheduledTasksCounter;
        m_TaskQueue.emplace(std::move(ThreadPoolTask{++m_taskCounter, data, func}));
        task = &m_TaskQueue.front();
    }

    m_threadEventVar.notify_one();

    return *task;
}

void ThreadPool::flush()
{
    while(!empty()) {};
}

void ThreadPool::start()
{
    for(auto i = 0; i < m_maxThreadCount; ++i)
    {
        m_threads.emplace_back([=] {
            while(true)
            {
                ThreadPoolTask* pTask = nullptr;
                {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    m_threadEventVar.wait(lock, [=] { return m_stopping || !m_TaskQueue.empty(); });

                    if (m_stopping)
                        break;

                    if(!m_TaskQueue.empty()) {
                        pTask = &m_TaskQueue.front();
                        m_TaskQueue.pop();
                    }
                }

                if(pTask) {
                    pTask->func(0, pTask->data);
                    --m_scheduledTasksCounter;
                }
            }
        });
    }
}

void ThreadPool::stop()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stopping = true;
    }

    m_threadEventVar.notify_all();

    for(auto& thread : m_threads) {
        if (thread.joinable())
            thread.join();
    }
}

const bool ThreadPool::empty() const
{
    return m_scheduledTasksCounter == 0;
}