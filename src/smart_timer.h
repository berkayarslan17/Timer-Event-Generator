#ifndef SMART_TIMER
#define SMART_TIMER

#include "base_timer.h"
#include <iostream>
#include <atomic>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "event_queue.h"

using operation = std::function<void()>;

class smart_timer : public base_timer
{
private:
    double val;
    dispatch_q dispatch_queue;
    std::atomic<bool> done;
    std::thread *runnable;

public:
    smart_timer();
    ~smart_timer(); 
    double get_val();
    void run();
};

#endif