#ifndef _TIMER_H
#define _TIMER_H

#include <chrono>
#include <array>
#include <thread>
#include <map>
#include <functional>

#define QUEUE_SIZE 200

using CLOCK = std::chrono::high_resolution_clock;
using timer_callback = std::function<void()>;
using millisecs = std::chrono::milliseconds;
using timepoint = CLOCK::time_point;
using predicate = std::function<bool()>;

typedef enum event_type
{
    REGISTER_EVENT_1,
    REGISTER_EVENT_2,
    REGISTER_EVENT_3,
    REGISTER_EVENT_4
} event_t;

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
    millisecs schedule_time(const timer_callback &cb);

    void handle_timer_events();

private:
    std::map<timer_member, event_t> hash_table;
    std::thread *runnable;
};

class timer_member
{
public:
    timer_member(const timepoint &tp, const timer_callback &cb);
    timer_member(const millisecs &period, const timer_callback &cb);
    timer_member(const timepoint &tp, const millisecs &period, const timer_callback &cb);
    timer_member(const predicate &pred, const millisecs &period, const timer_callback &cb);
    ~timer_member();

private:
    millisecs period;
    timer_callback cb;
    predicate pred;
    timepoint tp;
};

// class timer_event_handler
// {
// public:
//     timer_event_handler();
//     ~timer_event_handler();
//     int handle_reg_event_1(const timepoint &tp, const timer_callback &cb);
//     int handle_reg_event_2(const millisecs &period, const timer_callback &cb);
//     int handle_reg_event_3(const timepoint &tp, const millisecs &period, const timer_callback &cb);
//     int handle_reg_event_4(const predicate &pred, const millisecs &period, const timer_callback &cb);
//     void handle_event(event_t evt);
//     timer_callback get_handle(void) const
//     {
//         return schedule.get_handle();
//     }

// private:
// };
#endif