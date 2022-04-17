#include "timer.h"

/** @brief Constructor of the timer manager class
 *
 * @details In order to track the run time data with timepoint variable, differentiation
 *          of the time_since_epoch is taken.
 *
 * @return none
 */
my_timer::my_timer() : done(false)
{
    std::cout << "my_timer::Constructor\n";
    time_now = CLOCK::now();
    time_epoch = time_now.time_since_epoch().count();
    divider = pow(10, 6);
    runnable = new std::thread(&my_timer::handle_timer_events, this);
}

/** @brief Destructor of the timer manager class
 *
 * @details When the destructor called, it terminates the timer thread.
 *
 * @return none
 */
my_timer::~my_timer()
{
    std::cout << "my_timer::Destructor\n";
    done = true;
    scheduler_table.clear();
    runnable->join();
}

/** @brief Registration of TIMER TYPE 1 (One-shot Timer)
 *
 * @details In TIMER TYPE 1, timer_callback function should be called exactly at timepoint.
 *
 * @param timepoint It indicates the timer's timepoint.
 * @param callback It is a function pointer that triggers the callback function.
 * @return none
 */
void my_timer::register_timer(const timepoint &tp, const timer_callback &cb)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_DEBT));
    // Add to timer handler table with event type
    timer_member *tim_mem = new timer_member(tp, cb);
    register_scheduler_table(*tim_mem);
}

/** @brief Registration of TIMER TYPE 2 (Periodic Timer)
 *
 * @details In TIMER TYPE 2, timer_callback function should be called every period_ms constantly.
 *
 * @param period It indicates the timer's period.
 * @param callback It is a function pointer that triggers the callback function.
 * @return none
 */
void my_timer::register_timer(const millisecs &period, const timer_callback &cb)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_DEBT));
    // Add to timer handler table with event type
    timer_member *tim_mem = new timer_member(period, cb);
    register_scheduler_table(*tim_mem);
}

/** @brief Registration of TIMER TYPE 3
 *
 * @details In TIMER TYPE 3, timer_callback function should be called if timepoint_ms is bigger than period_ms.
 *
 * @param timepoint It indicates the timer's timepoint.
 * @param period It indicates the timer's period.
 * @param callback It is a function pointer that triggers the callback function.
 * @return none
 */
void my_timer::register_timer(const timepoint &tp, const millisecs &period, const timer_callback &cb)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_DEBT));
    // Add to timer handler table with event type
    timer_member *tim_mem = new timer_member(tp, period, cb);
    register_scheduler_table(*tim_mem);
}

/** @brief Registration of TIMER TYPE 4
 *
 * @details In TIMER TYPE 4, timer_callback function should be called if predicate returns true.
 *
 * @param predicate It is a function pointer that triggers the specific lambda expression.
 * @param period It indicates the timer's period.
 * @param callback It is a function pointer that triggers the callback function.
 * @return none
 */
void my_timer::register_timer(const predicate &pred, const millisecs &period, const timer_callback &cb)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_DEBT));
    // Add to timer handler table with event type
    timer_member *tim_mem = new timer_member(pred, period, cb);
    register_scheduler_table(*tim_mem);
}

/** @brief Registration of the timer members to scheduler_table
 *
 * @details scheduler_table: It is a vector that keeps the timer member objects data. Two threads (main, timer) can manipulate
 *          that vector so it should be protected with mutual exclusion semaphores.
 *
 * @param timer_member member object of the timer manager class
 * @return none
 */
void my_timer::register_scheduler_table(timer_member &tim_mem)
{
    prescheduler(tim_mem);
    // Lock the mutex because of the scheduler_table manipulation
    std::lock_guard<std::mutex> lock(timer_mutex);
    scheduler_table.push_back(tim_mem);
    sort_scheduler_table();
}

/** @brief Preschedule the timer member to vector
 *
 * @details It sets the timer member's deadline according to its type after the registration.
 *
 * @param timer_member member object of the timer manager class
 * @return none
 */
void my_timer::prescheduler(timer_member &tim_mem)
{
    // We should save the deadlines related to each specific timer members.

    switch (tim_mem.timer_type)
    {
    case TIMER_TYPE_1:
    {
        auto timepoint = tim_mem.get_member_timepoint();
        auto tp_ms = timepoint.time_since_epoch().count();
        auto deadline_ms = (tp_ms - time_epoch) / divider;
        // Save the deadline
        tim_mem.set_member_deadline(deadline_ms);
        timer_sem.give();

        break;
    }

    case TIMER_TYPE_2:
    {
        auto period = tim_mem.get_member_period();
        auto deadline_ms = tim_mem.period_cnt * period.count();
        // Save the deadline
        tim_mem.set_member_deadline(deadline_ms);
        // Never remove this timer from table
        timer_sem.give();
        break;
    }

    case TIMER_TYPE_3:
    {
        auto period = tim_mem.get_member_period();
        auto deadline_ms = tim_mem.period_cnt * period.count();

        // Save the deadline
        tim_mem.set_member_deadline(deadline_ms);
        timer_sem.give();

        break;
    }

    case TIMER_TYPE_4:
    {
        auto period = tim_mem.get_member_period();
        auto deadline_ms = tim_mem.period_cnt * period.count();
        // Save the deadline
        tim_mem.set_member_deadline(deadline_ms);
        timer_sem.give();

        break;
    }

    default:
        std::cout << "Unknown timer type\n";
        break;
    }
}

/** @brief scheduler_table sorting function
 *
 * @return none
 */
void my_timer::sort_scheduler_table(void)
{
    // Sort the deadline vector, return the earliest deadline.
    if (scheduler_table.size() > 1)
    {
        std::sort(scheduler_table.begin(), scheduler_table.end(), [](timer_member &lhs, timer_member &rhs)
                  { return lhs.get_member_deadline() < rhs.get_member_deadline(); });
    }
}

/** @brief Compute sleep for timer thread
 *
 * @details Computation of sleep depends on the time_past variable. It basically indicates how much time passed when one timer
 *          member preempt the other member in the timer thread.
 *
 * @return result
 */
long long compute_sleep(long long sleep, long long time_past)
{
    long long result = sleep - time_past;
    return result;
}

/** @brief Schedule the timer member to vector
 *
 * @details It controls the timer members' conditions based on their type in order to decide whether they should be removed from the table or not.
 *
 * @return none
 */
void my_timer::scheduler(void)
{
    auto tim_mem = scheduler_table.begin();
    switch (tim_mem->timer_type)
    {
    case TIMER_TYPE_1:
    {
        // Remove the timer from schedule table
        scheduler_table.erase(tim_mem);
        break;
    }

    // Never remove this timer from table
    case TIMER_TYPE_2:
    {
        auto period = tim_mem->get_member_period();
        tim_mem->period_cnt++;
        auto deadline_ms = tim_mem->period_cnt * period.count();
        // Save the deadline
        tim_mem->set_member_deadline(deadline_ms);
        // Give semaphore for triggering thread again
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
            scheduler_table.erase(tim_mem);
        }
        else
        {
            // Give semaphore for triggering thread again
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
            scheduler_table.erase(scheduler_table.begin());
        }

        else
        {
            auto period = tim_mem->get_member_period();
            tim_mem->period_cnt++;
            auto deadline_ms = tim_mem->period_cnt * period.count();
            // Save the deadlinde
            tim_mem->set_member_deadline(deadline_ms);
            // Give semaphore for triggering thread again
            timer_sem.give();
        }

        break;
    }

    default:
        std::cout << "Unknown timer type\n";
        break;
    }
}

/** @brief Timer thread
 *
 * @details It manages the whole timer events until the destructor of the timer will be called.
 * 
 * @return none
 */
void my_timer::handle_timer_events(void)
{
    long long time_past = DELAY_DEBT;

    while (!done)
    {
        while (!scheduler_table.empty())
        {
            timer_sem.take();
            // Take the timer member that has the closest deadline.
            timer_member tim_mem = scheduler_table.front();
            // Compute the sleep data according to time_past variable.
            auto ms_sleep = compute_sleep(tim_mem.get_member_deadline(), time_past);
            // Sleep the thread with short interval and check out whether the timer queue changes or not.
            std::chrono::duration<double, std::milli> sleep(ms_sleep / SLEEP_CNT);

            for (size_t i = 1; i <= SLEEP_CNT; i++)
            {
                // get size of the timer queue
                auto old_size = scheduler_table.size();
                std::this_thread::sleep_for(sleep);
                auto new_size = scheduler_table.size();

                // check the queue whether it is changed or not.
                if (old_size != new_size)
                {
                    time_past += sleep.count() * i;
                    timer_sem.give();
                    break;
                }

                // Trigger the callback function when the for loop reaches its ending.
                if (i == SLEEP_CNT)
                {
                    time_past = tim_mem.get_member_deadline();
                    // Trigger the callback function
                    tim_mem.get_member_cb()();
                    // Lock the mutex because of the scheduler_table manipulation
                    std::lock_guard<std::mutex> lock(timer_mutex);
                    scheduler();
                    sort_scheduler_table();
                }
            }
        }
    }
}

/** @brief Constructor of TIMER TYPE 1 member
 *
 * @param timepoint It indicates the timer's timepoint.
 * @param callback It is a function pointer that triggers the callback function.
 */
timer_member::timer_member(const timepoint &tp, const timer_callback &cb)
{
    set_member_timepoint(tp);
    set_member_cb(cb);

    timer_type = TIMER_TYPE_1;
}

/** @brief Constructor of TIMER TYPE 2 member
 *
 * @param period It indicates the timer's period.
 * @param callback It is a function pointer that triggers the callback function.
 */
timer_member::timer_member(const millisecs &period, const timer_callback &cb)
{
    set_member_period(period);
    set_member_cb(cb);

    timer_type = TIMER_TYPE_2;
    period_cnt = 1;
}

/** @brief Constructor of TIMER TYPE 3 member
 *
 * @param timepoint It indicates the timer's timepoint.
 * @param period It indicates the timer's period.
 * @param callback It is a function pointer that triggers the callback function.
 */
timer_member::timer_member(const timepoint &tp, const millisecs &period, const timer_callback &cb)
{
    set_member_timepoint(tp);
    set_member_period(period);
    set_member_cb(cb);

    timer_type = TIMER_TYPE_3;
    period_cnt = 1;
}

/** @brief Constructor of TIMER TYPE 4 member
 *
 * @param predicate It is a function pointer that triggers the specific lambda expression.
 * @param period It indicates the timer's period.
 * @param callback It is a function pointer that triggers the callback function.
 */
timer_member::timer_member(const predicate &pred, const millisecs &period, const timer_callback &cb)
{
    set_member_predicate(pred);
    set_member_period(period);
    set_member_cb(cb);

    timer_type = TIMER_TYPE_4;
    period_cnt = 1;
}

/** @brief Destructor of timer member object
 *
 * @return none
 */
timer_member::~timer_member()
{
}

/** @brief Set period to timer member object
 *
 * @return none
 */
void timer_member::set_member_period(const millisecs &period)
{
    mem_period = period;
}

/** @brief Set callback to time member object 
 *
 * @return none
 */
void timer_member::set_member_cb(const timer_callback &cb)
{
    mem_cb = cb;
}

/** @brief Set predicate to timer member object 
 *
 * @return none
 */
void timer_member::set_member_predicate(const predicate &pred)
{
    mem_pred = pred;
}

/** @brief Set timepoint to timer member object 
 *
 * @return none
 */
void timer_member::set_member_timepoint(const timepoint &tp)
{
    mem_tp = tp;
}

/** @brief Set deadline to timer member object 
 *
 * @return none
 */
void timer_member::set_member_deadline(const long long &deadline)
{
    mem_deadline = deadline;
}

/** @brief Get period from timer member object
 *
 * @return period
 */
millisecs timer_member::get_member_period()
{
    return mem_period;
}

/** @brief Get callback from timer member 
 *
 * @return callback
 */
timer_callback timer_member::get_member_cb()
{
    return mem_cb;
}

/** @brief Get predicate from timer member 
 *
 * @return predicate
 */
predicate timer_member::get_member_predicate()
{
    return mem_pred;
}

/** @brief Get timepoint from timer member 
 *
 * @return timepoint
 */
timepoint timer_member::get_member_timepoint()
{
    return mem_tp;
}

/** @brief Get deadline from timer member
 *
 * @return deadline
 */
long long timer_member::get_member_deadline()
{
    return mem_deadline;
}