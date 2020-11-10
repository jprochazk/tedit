#include "pch.h"

#ifndef TEDIT_TASK_
#define TEDIT_TASK_

#include <moodycamel/concurrentqueue.h>

using Task = std::function<void()>;

class TaskQueue
{
public:
    TaskQueue();

    void enqueue(Task&& task);
    bool empty();
    void poll();
    /* void poll_one(); */

private:
    std::atomic_size_t count_;
    moodycamel::ConcurrentQueue<Task> queue_;
}; // class TaskQueue

#endif // TEDIT_TASK_
