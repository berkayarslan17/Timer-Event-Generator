#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <mutex>
#include <queue>

using operation = std::function<void()>;

class dispatch_q
{
    std::mutex qlock;
    std::queue<operation> ops_queue;
    std::condition_variable empty;

public:
    void put(operation op);
    operation take();
};

#endif