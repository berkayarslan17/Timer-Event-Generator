#include "timer.h"

my_timer::my_timer() : done(false), ready(false)
{
    std::cout << "my_timer::Constructor\n";
    time_now = CLOCK::now();
    time_epoch = time_now.time_since_epoch().count();
    divider = pow(10, 6);
    runnable = new std::thread(&my_timer::handle_timer_events, this);
}

my_timer::~my_timer()
{
    std::cout << "my_timer::Destructor\n";
    done = true;
    schedule_table.clear();
    runnable->join();
}

void my_timer::register_timer(const timepoint &tp, const timer_callback &cb)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_DEBT));
    // Add to timer handler table with event type
    std::unique_ptr<timer_member> tim_mem = std::make_unique<timer_member>(tp, cb);
    // timer_member *tim_mem = new timer_member(tp, cb);
    register_scheduler_table(*tim_mem);
}

void my_timer::register_timer(const millisecs &period, const timer_callback &cb)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_DEBT));
    // Add to timer handler table with event type
    std::unique_ptr<timer_member> tim_mem = std::make_unique<timer_member>(period, cb);
    // timer_member *tim_mem = new timer_member(period, cb);
    register_scheduler_table(*tim_mem);
}

void my_timer::register_timer(const timepoint &tp, const millisecs &period, const timer_callback &cb)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_DEBT));
    // Add to timer handler table with event type
    std::unique_ptr<timer_member> tim_mem = std::make_unique<timer_member>(tp, period, cb);
    // timer_member *tim_mem = new timer_member(tp, period, cb);
    register_scheduler_table(*tim_mem);
}

void my_timer::register_timer(const predicate &pred, const millisecs &period, const timer_callback &cb)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_DEBT));
    // Add to timer handler table with event type
    std::unique_ptr<timer_member> tim_mem = std::make_unique<timer_member>(pred, period, cb);
    // timer_member *tim_mem = new timer_member(pred, period, cb);
    register_scheduler_table(*tim_mem);
}

// Find the earliest deadline among the timer member variables.
void my_timer::register_scheduler_table(timer_member &tim_mem)
{
    compute_deadline(tim_mem);
    std::lock_guard<std::mutex> lock(timer_mutex);
    schedule_table.push_back(tim_mem);
    sort_by_deadline();
}

// Find the earliest deadline among the timer member variables.
void my_timer::compute_deadline(timer_member &tim_mem)
{
    // We should save the deadlines related to each specific timer members.

    switch (tim_mem.timer_type)
    {
    case TIMER_TYPE_1:
    {
        auto timepoint = tim_mem.get_member_timepoint();
        auto tp_ms = timepoint.time_since_epoch().count();
        auto deadline_ms = (tp_ms - time_epoch) / divider;
        tim_mem.set_member_deadline(deadline_ms);
        // Save the timer's deadline to a vector
        timer_sem.give();

        break;
    }

    case TIMER_TYPE_2:
    {
        auto period = tim_mem.get_member_period();
        auto deadline_ms = tim_mem.period_cnt * period.count();
        // Save the deadline to vector
        tim_mem.set_member_deadline(deadline_ms);
        // Never remove this timer from table
        timer_sem.give();
        break;
    }

    case TIMER_TYPE_3:
    {
        auto period = tim_mem.get_member_period();
        auto deadline_ms = tim_mem.period_cnt * period.count();

        // Save the deadline to vector
        tim_mem.set_member_deadline(deadline_ms);
        timer_sem.give();

        break;
    }

    case TIMER_TYPE_4:
    {
        auto period = tim_mem.get_member_period();
        auto deadline_ms = tim_mem.period_cnt * period.count();
        // Check the predicate's value. If it is false, remove the timer from table
        // Save the deadline to vector
        tim_mem.set_member_deadline(deadline_ms);
        timer_sem.give();

        break;
    }

    default:
        std::cout << "Unknown timer type\n";
        break;
    }
}

void my_timer::sort_by_deadline(void)
{
    // Sort the deadline vector, return the earliest deadline.
    if (schedule_table.size() > 1)
    {
        std::sort(schedule_table.begin(), schedule_table.end(), [](timer_member &lhs, timer_member &rhs)
                  { return lhs.get_member_deadline() < rhs.get_member_deadline(); });
    }
}

long long compute_sleep(long long sleep, long long time_past)
{
    long long result = sleep - time_past;
    return result;
}

void my_timer::decide_timers_attitude(void)
{
    auto tim_mem = schedule_table.begin();
    switch (tim_mem->timer_type)
    {
    case TIMER_TYPE_1:
    {
        // Remove the timer from schedule table
        schedule_table.erase(tim_mem);
        break;
    }

    case TIMER_TYPE_2:
    {
        // Never remove this timer from table
        // Give semaphore for triggering thread again
        auto period = tim_mem->get_member_period();
        tim_mem->period_cnt++;
        auto deadline_ms = tim_mem->period_cnt * period.count();
        // Save the deadline to vector
        tim_mem->set_member_deadline(deadline_ms);
        // Never remove this timer from table
        timer_sem.give();
        break;
    }

    case TIMER_TYPE_3:
    {

        auto timepoint = tim_mem->get_member_timepoint();
        auto tp_ms = timepoint.time_since_epoch().count();
        auto threshold_ms = (tp_ms - time_epoch) / divider;
        auto period = tim_mem->get_member_period();
        tim_mem->period_cnt++;
        auto deadline_ms = tim_mem->period_cnt * period.count();
        tim_mem->set_member_deadline(deadline_ms);

        // Check the values' size. If threshold_ms smaller than deadline_ms, remove the timer from table
        if (threshold_ms <= deadline_ms)
        {
            // Remove the timer from table
            schedule_table.erase(tim_mem);
        }
        else
        {
            timer_sem.give();
        }

        break;
    }

    case TIMER_TYPE_4:
    {
        auto pred = tim_mem->get_member_predicate();
        // Check the predicate's value. If it is false, remove the timer from table
        if (!pred())
        {
            // Remove the timer from table
            schedule_table.erase(schedule_table.begin());
        }

        else
        {
            auto period = tim_mem->get_member_period();
            tim_mem->period_cnt++;
            auto deadline_ms = tim_mem->period_cnt * period.count();
            tim_mem->set_member_deadline(deadline_ms);
            timer_sem.give();
        }

        break;
    }

    default:
        std::cout << "Unknown timer type\n";
        break;
    }
}

void my_timer::handle_timer_events(void)
{
    // This is the scope that schedules the timer callback deadlines
    long long time_past = DELAY_DEBT;

    while (!done)
    {
        while (!schedule_table.empty())
        {
            timer_sem.take();
            timer_member tim_mem = schedule_table.front();
            // Wait for signals that will indicate the timer members are ready.
            auto ms_sleep = compute_sleep(tim_mem.get_member_deadline(), time_past);
            // Sleep the thread with short interval and check out whether the timer queue changes or not.
            std::chrono::duration<double, std::milli> sleep(ms_sleep / SLEEP_CNT);

            for (size_t i = 1; i <= SLEEP_CNT; i++)
            {
                // get size of the timer queue
                auto old_size = schedule_table.size();
                std::this_thread::sleep_for(sleep);
                auto new_size = schedule_table.size();

                // check the queue whether it is changed or not.
                if (old_size != new_size)
                {
                    time_past += sleep.count() * i;
                    timer_sem.give();
                    break;
                }

                if (i == SLEEP_CNT)
                {
                    time_past = tim_mem.get_member_deadline();
                    // Trigger the callback function
                    tim_mem.get_member_cb()();
                    std::lock_guard<std::mutex> lock(timer_mutex);
                    decide_timers_attitude();
                    sort_by_deadline();
                }
            }
        }
    }
}

timer_member::timer_member(const timepoint &tp, const timer_callback &cb)
{
    set_member_timepoint(tp);
    set_member_cb(cb);

    timer_type = TIMER_TYPE_1;
}

timer_member::timer_member(const millisecs &period, const timer_callback &cb)
{
    set_member_period(period);
    set_member_cb(cb);

    timer_type = TIMER_TYPE_2;
    period_cnt = 1;
}

timer_member::timer_member(const timepoint &tp, const millisecs &period, const timer_callback &cb)
{
    set_member_timepoint(tp);
    set_member_period(period);
    set_member_cb(cb);

    timer_type = TIMER_TYPE_3;
    period_cnt = 1;
}

timer_member::timer_member(const predicate &pred, const millisecs &period, const timer_callback &cb)
{
    set_member_predicate(pred);
    set_member_period(period);
    set_member_cb(cb);

    timer_type = TIMER_TYPE_4;
    period_cnt = 1;
}

timer_member::~timer_member()
{
}

void timer_member::set_member_period(const millisecs &period)
{
    mem_period = period;
}

void timer_member::set_member_cb(const timer_callback &cb)
{
    mem_cb = cb;
}

void timer_member::set_member_predicate(const predicate &pred)
{
    mem_pred = pred;
}

void timer_member::set_member_timepoint(const timepoint &tp)
{
    mem_tp = tp;
}

void timer_member::set_member_deadline(const long long &deadline)
{
    mem_deadline = deadline;
}

millisecs timer_member::get_member_period()
{
    return mem_period;
}

timer_callback timer_member::get_member_cb()
{
    return mem_cb;
}

predicate timer_member::get_member_predicate()
{
    return mem_pred;
}

timepoint timer_member::get_member_timepoint()
{
    return mem_tp;
}

long long timer_member::get_member_deadline()
{
    return mem_deadline;
}