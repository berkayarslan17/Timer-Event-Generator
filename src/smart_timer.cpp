#include "smart_timer.h"

smart_timer::smart_timer() : val(0), done(false)
{
    runnable = new std::thread(&smart_timer::run, this);
}

smart_timer::~smart_timer()
{
    runnable->join();
}

double smart_timer::get_val()
{
    return val;
}

void smart_timer::run()
{
    while (!done)
    {
        dispatch_queue.take()();
    }
}