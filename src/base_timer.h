#ifndef BASE_TIMER_H
#define BASE_TIMER_H

#include <chrono>
#include <functional>

using CLOCK = std::chrono::high_resolution_clock;
using timer_callback = std::function<void()>;
using millisecs = std::chrono::milliseconds;
using timepoint = CLOCK::time_point;
using predicate = std::function<bool()>;

class base_timer
{
public:
    virtual void register_timer(const timepoint &tp, const timer_callback &cb) = 0;
    virtual void register_timer(const millisecs &period, const timer_callback &cb) = 0;
    virtual void register_timer(const timepoint &tp, const millisecs &period, const timer_callback &cb) = 0;
    virtual void register_timer(const predicate &pred, const millisecs &period, const timer_callback &cb) = 0;
};

#endif