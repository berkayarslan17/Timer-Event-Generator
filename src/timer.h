#ifndef _TIMER_H
#define _TIMER_H

#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <map>
#include <functional>
#include <algorithm>

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
} timer_t;

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
    void compute_deadline(void);
    void handle_timer_events();

private:
    std::vector<timer_member> schedule_table;
    std::vector<std::pair<double, timer_member>> deadline_table;
    std::thread *runnable;
    double time_epoch;
};

class timer_member
{
public:
    timer_member(const timepoint &tp, const timer_callback &cb);
    timer_member(const millisecs &period, const timer_callback &cb);
    timer_member(const timepoint &tp, const millisecs &period, const timer_callback &cb);
    timer_member(const predicate &pred, const millisecs &period, const timer_callback &cb);
    ~timer_member();
    void set_member_period(const millisecs &period);
    void set_member_cb(const timer_callback &cb);
    void set_member_predicate(const predicate &pred);
    void set_member_timepoint(const timepoint &tp);
    millisecs get_member_period();
    timer_callback get_member_cb();
    predicate get_member_predicate();
    timepoint get_member_timepoint();

    int period_cnt;
    double deadline;
    timer_t timer_type;

private:
    millisecs mem_period;
    timer_callback mem_cb;
    predicate mem_pred;
    timepoint mem_tp;
};

#endif