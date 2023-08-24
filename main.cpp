#include <iostream>
#include <chrono>
#include <assert.h>
#include "ThreadPool/ThreadPool.h"

void testFunc(uint64_t taskId, void* data)
{
    std::chrono::high_resolution_clock clock;
    auto startTime = clock.now();
    do
    {
        auto currentTime = clock.now();
        std::chrono::duration<double> elapsedTime = duration_cast<std::chrono::milliseconds>(currentTime - startTime);
        if(elapsedTime.count() > 5)
            break;
    } while(true);
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
