#include <iostream>
#include <cmath>
#include <chrono>
#include "timer.h"

static CLOCK::time_point T0;
std::mutex timer_mutex;

void log_callback(int id, const std::string &logstr)
{
    auto dt = CLOCK::now() - T0;
    std::cout << "[" << std::chrono::duration_cast<std::chrono::milliseconds>(dt).count()
              << "] (cb " << id << "): " << logstr << std::endl;
}

int main()
{
    // determine time_epoch
    // auto tp = CLOCK::now();
    // auto time_epoch = tp.time_since_epoch().count();
    // // create timepoint variable
    // auto t1 = CLOCK::now() + std::chrono::seconds(2);
    // auto t1_ms = t1.time_since_epoch().count();
    // auto deadline_ms = (t1_ms - time_epoch) / divider;

    // std::cout << "time_epoch: " << time_epoch << " ms\n";
    // std::cout << "t1_ms: " << t1_ms << " ms\n";
    // std::cout << "deadline_ms: " << deadline_ms << " ms\n";
    // subtract timepoint and time_epoch

    my_timer timer;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    T0 = CLOCK::now();

    log_callback(-1, "main starting");
    auto t1 = CLOCK::now() + std::chrono::milliseconds(300);
    auto t2 = t1 + std::chrono::seconds(1);
    auto t3 = t2 + std::chrono::seconds(3);
    auto t4 = t2 + std::chrono::seconds(2);

    /* TIMER TYPE 1 */

    timer.register_timer(t2, [&]()
                         { log_callback(1, "callback str"); });
    timer.register_timer(t1, [&]()
                         { log_callback(2, "callback str"); });
    timer.register_timer(t3, [&]()
                         { log_callback(3, "callback str"); });
    timer.register_timer(t4, [&]()
                         { log_callback(4, "callback str"); });

    /* TIMER TYPE 2 */

    timer.register_timer(millisecs(1000), [&]()
                         { log_callback(5, "callback str"); });

    /* TIMER TYPE 3 */

    timer.register_timer(t1, millisecs(1000), [&]()
                         { log_callback(6, "callback str"); });

    /* TIMER TYPE 4 */

    timer.register_timer([&]()
                         {static int count = 0; return ++count < 3; },
                         millisecs(500), [&]()
                         { log_callback(7, "callback str"); });

    return 0;
}