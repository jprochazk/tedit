#include "pch.h"
#include "task.hpp"

template<typename T>
std::vector<T>
dequeue_all(moodycamel::ConcurrentQueue<T>& queue, std::size_t count)
{
    std::vector<T> out;
    out.resize(count, T{});
    queue.try_dequeue_bulk(out.begin(), count);
    return out;
}

TaskQueue::TaskQueue()
  : count_(0)
  , queue_()
{}

void
TaskQueue::enqueue(Task&& task)
{
    ++count_;
    queue_.enqueue(std::move(task));
}

bool
TaskQueue::empty()
{
    return count_ == 0;
}

void
TaskQueue::poll()
{
    if (!this->empty()) {
        size_t count = this->count_;
        for (size_t i = 0; i < count; ++i) {
            Task task;
            if (this->queue_.try_dequeue(task)) {
                task();
            }
        }
    }
}