#ifndef _TIMER_H
#define _TIMER_H

#include <iostream>
#include <atomic>
#include <cmath>
#include <chrono>
#include <vector>
#include <thread>
#include <functional>
#include <algorithm>
#include <condition_variable>

#define SLEEP_CNT 10
#define DELAY_DEBT 10

using CLOCK = std::chrono::high_resolution_clock;
using timer_callback = std::function<void()>;
using millisecs = std::chrono::milliseconds;
using timepoint = CLOCK::time_point;
using predicate = std::function<bool()>;

typedef enum timer_type
{
    TIMER_TYPE_1,
    TIMER_TYPE_2,
    TIMER_TYPE_3,
    TIMER_TYPE_4,
} timer_type_t;

class semaphore
{
    std::mutex mutex_;
    std::condition_variable condition_;
    unsigned long count_ = 0; // Initialized as locked.

public:
    void give()
    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        ++count_;
        condition_.notify_one();
    }

    void take()
    {
        std::unique_lock<decltype(mutex_)> lock(mutex_);
        while (!count_) // Handle spurious wake-ups.
            condition_.wait(lock);
        --count_;
    }

    bool try_take()
    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        if (count_)
        {
            --count_;
            return true;
        }
        return false;
    }
};

class timer_member
{
public:
    timer_member();
    timer_member(const timepoint &tp, const timer_callback &cb);
    timer_member(const millisecs &period, const timer_callback &cb);
    timer_member(const timepoint &tp, const millisecs &period, const timer_callback &cb);
    timer_member(const predicate &pred, const millisecs &period, const timer_callback &cb);
    ~timer_member();
    void set_member_period(const millisecs &period);
    void set_member_cb(const timer_callback &cb);
    void set_member_predicate(const predicate &pred);
    void set_member_timepoint(const timepoint &tp);
    void set_member_deadline(const long long &deadline);
    millisecs get_member_period();
    timer_callback get_member_cb();
    predicate get_member_predicate();
    timepoint get_member_timepoint();
    long long get_member_deadline();

    int period_cnt;
    timer_type_t timer_type;
    long long mem_deadline;

private:
    millisecs mem_period;
    timer_callback mem_cb;
    predicate mem_pred;
    timepoint mem_tp;
};

class base_timer
{
public:
    virtual void register_timer(const timepoint &tp, const timer_callback &cb) = 0;
    virtual void register_timer(const millisecs &period, const timer_callback &cb) = 0;
    virtual void register_timer(const timepoint &tp, const millisecs &period, const timer_callback &cb) = 0;
    virtual void register_timer(const predicate &pred, const millisecs &period, const timer_callback &cb) = 0;
};

class my_timer : public base_timer
{

public:
    my_timer();
    ~my_timer();
    void register_timer(const timepoint &tp, const timer_callback &cb);
    void register_timer(const millisecs &period, const timer_callback &cb);
    void register_timer(const timepoint &tp, const millisecs &period, const timer_callback &cb);
    void register_timer(const predicate &pred, const millisecs &period, const timer_callback &cb);

    void register_scheduler_table(timer_member &tim_mem);
    void prescheduler(timer_member &tim_mem);
    void sort_scheduler_table(void);
    void scheduler(void);
    void handle_timer_events(void);

private:
    std::vector<timer_member> scheduler_table;
    std::mutex timer_mutex;
    semaphore timer_sem;
    std::thread *runnable;
    std::atomic<bool> done;
    timepoint time_now;
    long long time_epoch;
    long long divider;
};

#endif