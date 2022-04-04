#include "timer.h"

my_timer::my_timer()
{
    runnable = new std::thread(&my_timer::handle_timer_events, this);
}

my_timer::~my_timer()
{
    runnable->join();
}

void my_timer::register_timer(const timepoint &tp, const timer_callback &cb)
{
    // Add to timer handler table with event type
    timer_member tim_mem = timer_member(tp, cb);
    hash_table.insert({tim_mem, REGISTER_EVENT_1});
}

void my_timer::register_timer(const millisecs &period, const timer_callback &cb)
{
    // Add to timer handler table with event type
    timer_member tim_mem = timer_member(period, cb);
    hash_table.insert({tim_mem, REGISTER_EVENT_2});
}

void my_timer::register_timer(const timepoint &tp, const millisecs &period, const timer_callback &cb)
{
    // Add to timer handler table with event type
    timer_member tim_mem = timer_member(tp, period, cb);
    hash_table.insert({tim_mem, REGISTER_EVENT_3});
}

void my_timer::register_timer(const predicate &pred, const millisecs &period, const timer_callback &cb)
{
    // Add to timer handler table with event type
    timer_member tim_mem = timer_member(pred, period, cb);
    hash_table.insert({tim_mem, REGISTER_EVENT_4});
}

// Find the earliest deadline among the timer member variables.
millisecs my_timer::schedule_time(const timer_callback &cb)
{
}

void my_timer::handle_timer_events()
{
    // This is the scope that schedules the timer callback deadlines
    timer_callback cb;
    millisecs block_time;

    for (;;)
    {
        // sleep the thread with block_time
        block_time = schedule_time(cb);
        std::this_thread::sleep_for(block_time);
    }
}