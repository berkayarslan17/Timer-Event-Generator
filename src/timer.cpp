#include "timer.h"

my_timer::my_timer()
{
    std::cout << "Task::Constructor\n";
    auto timepoint = std::chrono::system_clock::now();
    time_epoch = timepoint.time_since_epoch().count();
    int divider = pow(10, 6);
    runnable = new std::thread(&my_timer::handle_timer_events, this);
}

my_timer::~my_timer()
{
    std::cout << "Task::Destructor\n";
    runnable->join();
}

void my_timer::register_timer(const timepoint &tp, const timer_callback &cb)
{
    // Add to timer handler table with event type
    std::unique_ptr<timer_member> tim_mem = std::make_unique<timer_member>(tp, cb);
    register_scheduler_table(*tim_mem);
}

void my_timer::register_timer(const millisecs &period, const timer_callback &cb)
{
    // Add to timer handler table with event type
    std::unique_ptr<timer_member> tim_mem = std::make_unique<timer_member>(period, cb);
    register_scheduler_table(*tim_mem);
}

void my_timer::register_timer(const timepoint &tp, const millisecs &period, const timer_callback &cb)
{
    // Add to timer handler table with event type
    std::unique_ptr<timer_member> tim_mem = std::make_unique<timer_member>(tp, period, cb);
    register_scheduler_table(*tim_mem);
}

void my_timer::register_timer(const predicate &pred, const millisecs &period, const timer_callback &cb)
{
    // Add to timer handler table with event type
    std::unique_ptr<timer_member> tim_mem = std::make_unique<timer_member>(pred, period, cb);
    register_scheduler_table(*tim_mem);
}

// Find the earliest deadline among the timer member variables.
void my_timer::register_scheduler_table(timer_member &tim_mem)
{
    schedule_table.push_back(tim_mem);
    compute_deadline();
}

// Find the earliest deadline among the timer member variables.
void my_timer::compute_deadline(void)
{
    // We should save the deadlines related to each specific timer members.
    for (auto tim_mem = schedule_table.begin(); tim_mem != schedule_table.end(); ++tim_mem)
    {
        std::lock_guard<std::mutex> guard(timer_mutex);

        switch (tim_mem->timer_type)
        {
        case TIMER_TYPE_1:
        {
            auto timepoint = tim_mem->get_member_timepoint();
            auto tp_ms = timepoint.time_since_epoch().count();
            auto deadline_ms = (tp_ms - time_epoch) / divider;
            // Save the timer's deadline to a vector
            deadline_table.push_back(std::make_pair(deadline_ms, *tim_mem));
            // Remove the timer from schedule table
            schedule_table.erase(tim_mem);

            break;
        }

        case TIMER_TYPE_2:
        {
            auto period = tim_mem->get_member_period();
            auto deadline_ms = tim_mem->period_cnt * period.count() - time_epoch;
            // Save the deadline to vector
            deadline_table.push_back(std::make_pair(deadline_ms, *tim_mem));
            tim_mem->period_cnt++;
            // Never remove this timer from table
            break;
        }

        case TIMER_TYPE_3:
        {
            auto timepoint = tim_mem->get_member_timepoint();
            auto tp_ms = std::chrono::time_point_cast<millisecs>(timepoint).time_since_epoch().count();
            auto threshold_ms = tp_ms - time_epoch;

            auto period = tim_mem->get_member_period();
            auto deadline_ms = tim_mem->period_cnt * period.count() - time_epoch;

            // Check the values' size. If threshold_ms smaller than deadline_ms, remove the timer from table
            if (threshold_ms < deadline_ms)
            {
                // Remove the timer from table
                schedule_table.erase(tim_mem);
            }

            else
            {
                // Save the deadline to vector
                deadline_table.push_back(std::make_pair(deadline_ms, *tim_mem));
                tim_mem->period_cnt++;
            }

            break;
        }

        case TIMER_TYPE_4:
        {
            auto pred = tim_mem->get_member_predicate();
            auto period = tim_mem->get_member_period();
            auto deadline_ms = tim_mem->period_cnt * period.count() - time_epoch;
            // Check the predicate's value. If it is false, remove the timer from table
            if (!pred)
            {
                // Remove the timer from table
                schedule_table.erase(tim_mem);
            }
            else
            {
                // Save the deadline to vector
                deadline_table.push_back(std::make_pair(deadline_ms, *tim_mem));
                tim_mem->period_cnt++;
            }

            break;
        }

        default:
            std::cout << "Unknown timer type\n";
            break;
        }
    }
    // Sort the deadline vector, return the earliest deadline.
    std::sort(deadline_table.begin(), deadline_table.end());
}

double compute_sleep(double &sleep)
{
    static double temp;
    auto result = sleep - temp;
    temp = sleep;
    return result;
}

void my_timer::handle_timer_events()
{
    // This is the scope that schedules the timer callback deadlines

    while (!schedule_table.empty())
    {
        // sleep this thread with block_time
        // TODO: We should not blocking with a constant period.
        while (!deadline_table.empty())
        {
            std::lock_guard<std::mutex> guard(timer_mutex);

            std::pair<double, timer_member> tim_mem = deadline_table.front();
            auto deadline = tim_mem.first;
            auto ms_sleep = compute_sleep(deadline);
            std::chrono::duration<double, std::milli> sleep(ms_sleep);

            std::this_thread::sleep_for(sleep);
            // Trigger the callback function
            auto callback = tim_mem.second.get_member_cb();
            callback();
            auto find_member = std::find_if(deadline_table.begin(), deadline_table.end(),
                                            [&tim_mem](const std::pair<double, timer_member> &element)
                                            {
                                                if ((element.first == tim_mem.first) && (element.second == tim_mem.second))
                                                {
                                                    return element;
                                                }
                                            });
            deadline_table.erase(find_member);
        }
    }
}

timer_member::timer_member(const timepoint &tp, const timer_callback &cb)
{
    set_member_timepoint(tp);
    set_member_cb(cb);

    timer_type = TIMER_TYPE_1;
    period_cnt = 0;
    deadline = 0;
}

timer_member::timer_member(const millisecs &period, const timer_callback &cb)
{
    set_member_period(period);
    set_member_cb(cb);

    timer_type = TIMER_TYPE_2;
    period_cnt = 0;
    deadline = 0;
}

timer_member::timer_member(const timepoint &tp, const millisecs &period, const timer_callback &cb)
{
    set_member_timepoint(tp);
    set_member_period(period);
    set_member_cb(cb);

    timer_type = TIMER_TYPE_3;
    period_cnt = 0;
    deadline = 0;
}

timer_member::timer_member(const predicate &pred, const millisecs &period, const timer_callback &cb)
{
    set_member_predicate(pred);
    set_member_period(period);
    set_member_cb(cb);

    timer_type = TIMER_TYPE_4;
    period_cnt = 0;
    deadline = 0;
}

timer_member::~timer_member()
{
}

bool timer_member::operator==(const timer_member &tim_mem) const
{
    return *this == tim_mem;
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