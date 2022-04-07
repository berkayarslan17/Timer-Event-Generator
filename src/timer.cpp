#include "timer.h"

my_timer::my_timer() : done(false)
{
    std::cout << "my_timer::Constructor\n";
    auto timepoint = CLOCK::now();
    time_epoch = timepoint.time_since_epoch().count();
    divider = pow(10, 6);
    runnable = new std::thread(&my_timer::handle_timer_events, this);
}

my_timer::~my_timer()
{
    std::cout << "my_timer::Destructor\n";
    runnable->join();
}

void my_timer::register_timer(const timepoint &tp, const timer_callback &cb)
{
    std::cout << "Timer Type 1\n";
    // Add to timer handler table with event type
    // std::unique_ptr<timer_member> tim_mem = std::make_unique<timer_member>(tp, cb);
    timer_member *tim_mem = new timer_member(tp, cb);
    register_scheduler_table(*tim_mem);
}

void my_timer::register_timer(const millisecs &period, const timer_callback &cb)
{
    std::cout << "Timer Type 2\n";
    // Add to timer handler table with event type
    // std::unique_ptr<timer_member> tim_mem = std::make_unique<timer_member>(period, cb);
    timer_member *tim_mem = new timer_member(period, cb);
    register_scheduler_table(*tim_mem);
}

void my_timer::register_timer(const timepoint &tp, const millisecs &period, const timer_callback &cb)
{
    std::cout << "Timer Type 3\n";
    // Add to timer handler table with event type
    // std::unique_ptr<timer_member> tim_mem = std::make_unique<timer_member>(tp, period, cb);
    timer_member *tim_mem = new timer_member(tp, period, cb);
    register_scheduler_table(*tim_mem);
}

void my_timer::register_timer(const predicate &pred, const millisecs &period, const timer_callback &cb)
{
    std::cout << "Timer Type 4\n";
    // Add to timer handler table with event type
    // std::unique_ptr<timer_member> tim_mem = std::make_unique<timer_member>(pred, period, cb);
    timer_member *tim_mem = new timer_member(pred, period, cb);
    register_scheduler_table(*tim_mem);
}

// Find the earliest deadline among the timer member variables.
void my_timer::register_scheduler_table(timer_member &tim_mem)
{
    std::cout << "Saving the timer to scheduler\n";
    compute_deadline(tim_mem);
    schedule_table.push_back(tim_mem);
    sort_by_deadline();
}

// Find the earliest deadline among the timer member variables.
void my_timer::compute_deadline(timer_member &tim_mem)
{
    std::cout << "Computing deadline\n";
    // We should save the deadlines related to each specific timer members.
    // std::cout << "Trying to take mutex in main thread\n";
    // std::lock_guard<std::mutex> guard(timer_mutex);
    // std::cout << "Mutex has been taken from timer thread\n";

    switch (tim_mem.timer_type)
    {
    case TIMER_TYPE_1:
    {
        auto timepoint = tim_mem.get_member_timepoint();
        auto tp_ms = timepoint.time_since_epoch().count();
        std::cout << "Computing tp_ms: " << tp_ms << "\n";
        auto deadline_ms = (tp_ms - time_epoch) / divider;
        tim_mem.set_member_deadline(deadline_ms);
        // Save the timer's deadline to a vector
        std::cout << "tim deadline " << tim_mem.get_member_deadline() << "\n";
        // Remove the timer from schedule table
        // schedule_table.erase(tim_mem);

        break;
    }

    case TIMER_TYPE_2:
    {
        auto period = tim_mem.get_member_period();
        auto deadline_ms = tim_mem.period_cnt * period.count() - time_epoch;
        std::cout << "Computing deadline: " << deadline_ms << "\n";
        // Save the deadline to vector
        tim_mem.set_member_deadline(deadline_ms);
        tim_mem.period_cnt++;
        // Never remove this timer from table
        break;
    }

    case TIMER_TYPE_3:
    {
        auto timepoint = tim_mem.get_member_timepoint();
        auto tp_ms = std::chrono::time_point_cast<millisecs>(timepoint).time_since_epoch().count();
        auto threshold_ms = tp_ms - time_epoch;
        std::cout << "Computing threshold: " << threshold_ms << "\n";

        auto period = tim_mem.get_member_period();
        auto deadline_ms = tim_mem.period_cnt * period.count() - time_epoch;
        std::cout << "Computing deadline: " << deadline_ms << "\n";

        // Check the values' size. If threshold_ms smaller than deadline_ms, remove the timer from table
        if (threshold_ms < deadline_ms)
        {
            // Remove the timer from table
            // schedule_table.erase(tim_mem);
        }

        else
        {
            // Save the deadline to vector
            tim_mem.set_member_deadline(deadline_ms);
            tim_mem.period_cnt++;
        }

        break;
    }

    case TIMER_TYPE_4:
    {
        auto pred = tim_mem.get_member_predicate();
        auto period = tim_mem.get_member_period();
        auto deadline_ms = tim_mem.period_cnt * period.count() - time_epoch;
        std::cout << "Computing deadline: " << deadline_ms << "\n";
        // Check the predicate's value. If it is false, remove the timer from table
        if (!pred)
        {
            // Remove the timer from table
            // schedule_table.erase(tim_mem);
        }
        else
        {
            // Save the deadline to vector
            tim_mem.set_member_deadline(deadline_ms);
            tim_mem.period_cnt++;
        }

        break;
    }

    default:
        std::cout << "Unknown timer type\n";
        break;
    }

    std::cout << "Deadline computed\n";
}

void my_timer::sort_by_deadline(void)
{
    // Sort the deadline vector, return the earliest deadline.
    if (schedule_table.size() > 1)
    {
        std::cout << "Sorting the vector\n";
        std::sort(schedule_table.begin(), schedule_table.end(), [](timer_member &lhs, timer_member &rhs)
                  { return lhs.get_member_deadline() < rhs.get_member_deadline(); });
    }
}

long long compute_sleep(long long sleep)
{
    static long long temp = 0;
    long long result = sleep - temp;
    temp = sleep;
    return result;
}

void my_timer::handle_timer_events(void)
{
    // This is the scope that schedules the timer callback deadlines

    std::cout << "Hello from timer thread\n";
    std::unique_lock<std::mutex> lock(timer_mutex);
    while (!done)
    {
        while (!schedule_table.empty())
        {
            // sleep this thread with block_time
            // TODO: We should not blocking with a constant period.
            auto tim_mem = schedule_table.front();

            std::cout << "timer type: " << tim_mem.timer_type << "\n";
            std::cout << "timer deadline: " << tim_mem.get_member_deadline() << "\n";
            auto ms_sleep = compute_sleep(tim_mem.get_member_deadline());
            std::cout << "ms_sleep: " << ms_sleep << "\n";
            // Sleep the thread with short interval and check out whether the timer queue changes or not.
            std::chrono::duration<double, std::milli> sleep(ms_sleep);

            std::this_thread::sleep_for(sleep);
            // Trigger the callback function
            tim_mem.get_member_cb()();
        }
    }
}

timer_member::timer_member(const timepoint &tp, const timer_callback &cb)
{
    set_member_timepoint(tp);
    set_member_cb(cb);

    timer_type = TIMER_TYPE_1;
    period_cnt = 0;
}

timer_member::timer_member(const millisecs &period, const timer_callback &cb)
{
    set_member_period(period);
    set_member_cb(cb);

    timer_type = TIMER_TYPE_2;
    period_cnt = 0;
}

timer_member::timer_member(const timepoint &tp, const millisecs &period, const timer_callback &cb)
{
    set_member_timepoint(tp);
    set_member_period(period);
    set_member_cb(cb);

    timer_type = TIMER_TYPE_3;
    period_cnt = 0;
}

timer_member::timer_member(const predicate &pred, const millisecs &period, const timer_callback &cb)
{
    set_member_predicate(pred);
    set_member_period(period);
    set_member_cb(cb);

    timer_type = TIMER_TYPE_4;
    period_cnt = 0;
}

timer_member::~timer_member()
{
}

timer_member &timer_member::operator=(const timer_member &tim_mem)
{
    period_cnt = tim_mem.period_cnt;
    timer_type = tim_mem.timer_type;
    set_member_period(tim_mem.mem_period);
    set_member_cb(tim_mem.mem_cb);
    set_member_predicate(tim_mem.mem_pred);
    set_member_timepoint(tim_mem.mem_tp);
    set_member_deadline(tim_mem.mem_deadline);
    return *this;
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