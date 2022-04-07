#include <iostream>
#include <cmath>
#include <chrono>
// #include "timer.h"

using CLOCK = std::chrono::high_resolution_clock;
using timepoint = CLOCK::time_point;
using millisecs = std::chrono::milliseconds;

int divider = pow(10, 6);

int main()
{
    // determine time_epoch
    auto tp = CLOCK::now();
    auto time_epoch = tp.time_since_epoch().count();
    // create timepoint variable
    auto t1 = CLOCK::now() + std::chrono::seconds(2);
    auto t1_ms = t1.time_since_epoch().count();
    auto deadline_ms = (t1_ms - time_epoch) / divider;

    std::cout << "time_epoch: " << time_epoch << " ms\n";
    std::cout << "t1_ms: " << t1_ms << " ms\n";
    std::cout << "deadline_ms: " << deadline_ms << " ms\n";
    // subtract timepoint and time_epoch
    return 0;
}