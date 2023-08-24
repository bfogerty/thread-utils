#include <iostream>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <queue>
#include <chrono>
#include <assert.h>

struct Task
{
    uint64_t taskId;
    void* data;
    void (*func)(const uint64_t taskId, void* data);
};

// Derived from the implementation designed by the Coding Blacksmith
// https://www.youtube.com/watch?v=eWTGtp3HXiw&t=607s
class ThreadPool
{
public:
    ThreadPool(const size_t maxThreadCount) :
    m_maxThreadCount(maxThreadCount)
    , m_stopping(false)
    , m_scheduledTasksCounter(0)
    {
        start();
    }

    ~ThreadPool()
    {
        stop();
    }

    const Task& enqueue(void* data, void (*func)(const uint64_t taskId, void* data))
    {
        Task* task = nullptr;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            ++m_scheduledTasksCounter;
            m_TaskQueue.emplace(std::move(Task{0, data, func}));
            task = &m_TaskQueue.front();
        }

        m_threadEventVar.notify_one();

        return *task;
    }

    void flush()
    {
        while(!empty()) {};
    }

private:

    void start()
    {
        for(auto i = 0; i < m_maxThreadCount; ++i)
        {
            m_threads.emplace_back([=] {
                while(true)
                {
                    Task* pTask = nullptr;
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

    void stop()
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

    const bool empty() const
    {
        return m_scheduledTasksCounter == 0;
    }

    size_t m_maxThreadCount;
    bool m_stopping;
    std::mutex m_mutex;
    std::condition_variable m_threadEventVar;
    std::vector<std::thread> m_threads;
    std::queue<Task> m_TaskQueue;
    std::atomic<uint32_t> m_scheduledTasksCounter;
};

void testFunc(uint64_t taskId, void* data)
{
    int* pValue = reinterpret_cast<int*>(data);
    int v = *pValue;
    *pValue = v * v * v * v;

    std::chrono::high_resolution_clock clock;
    auto startTime = clock.now();
    do
    {
        auto currentTime = clock.now();
        std::chrono::duration<double> elapsedTime = duration_cast<std::chrono::milliseconds>(currentTime - startTime);
        if(elapsedTime.count() > 5)
            break;
    }while(true);
}

int main() {

    ThreadPool pool{10};

    std::vector<uint32_t> values;
    for(int i = 0; i < 10; ++i)
    {
        values.push_back(i);
    }

    std::chrono::high_resolution_clock clock;
    auto startTime = clock.now();

    for(int i = 0; i < values.size(); ++i)
    {
        pool.enqueue(&values[i], testFunc);
    }

    pool.flush();

    auto endTime = clock.now();
    std::chrono::duration<double> elapsedTime = duration_cast<std::chrono::milliseconds>(endTime - startTime);

    std::cout << "Running time = " << elapsedTime.count() << " ms." << std::endl;

    return 0;
}
