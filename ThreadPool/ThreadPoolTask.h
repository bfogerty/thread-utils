#ifndef _THREADPOOL_TASK_H
#define _THREADPOOL_TASK_H

struct ThreadPoolTask
{
    uint64_t taskId;
    void* data;
    void (*func)(const uint64_t taskId, void* data);
};

#endif //_THREADPOOL_TASK_H
