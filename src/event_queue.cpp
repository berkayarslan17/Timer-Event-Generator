#include "event_queue.h"

void dispatch_q::put(operation op)
{
    std::lock_guard<std::mutex> guard(qlock);
    ops_queue.push(op);
    empty.notify_one();
}
operation dispatch_q::take()
    {
        std::unique_lock<std::mutex> lock(qlock);
        empty.wait(lock, [&]
                   { return !ops_queue.empty(); });

        operation op = ops_queue.front();
        ops_queue.pop();
        return op;
    }