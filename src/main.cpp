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
    my_timer timer;
    // std::this_thread::sleep_for(std::chrono::seconds(1));
    T0 = CLOCK::now();

    log_callback(-1, "main starting");
    auto t1 = CLOCK::now() + std::chrono::seconds(1);
    auto t2 = t1 + std::chrono::seconds(1);

    timer.register_timer(t2, [&]()
                         { log_callback(1, "callback str"); });
    timer.register_timer(t1, [&]()
                         { log_callback(2, "callback str"); });
    timer.register_timer(millisecs(700), [&]()
                         { log_callback(3, "callback str"); });
    timer.register_timer(t1 + millisecs(300), millisecs(500), [&]()
                         { log_callback(4, "callback str"); });
    timer.register_timer([&]()
                         {static int count = 0; return ++count < 3; },
                         millisecs(500), [&]()
                         { log_callback(5, "callback str"); });

    std::this_thread::sleep_for(std::chrono::seconds(5));
    log_callback(-1, "main terminating");
}