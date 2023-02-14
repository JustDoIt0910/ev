//
// Created by zr on 23-2-14.
//

#include "Timer.h"

namespace ev::reactor
{
    std::atomic_uint64_t Timer::nextId = 1;

    Timer::Timer(TimerTask task, Timestamp when, double interval):
        task(std::move(task)),
        _expiration(when),
        interval(interval),
        repeat(interval > 0.0),
        _id(nextId++) {}

    void Timer::run() {task();}

    Timestamp Timer::expiration() {return _expiration;}

    Timer::TimerId Timer::id() const {return _id;}

    void Timer::restart()
    {
        if(!repeat)
            _expiration = Timestamp::invalid();
        else
        {
            _expiration = Timestamp::now();
            auto ms = static_cast<int64_t>(interval * Timestamp::microSecondsPerSecond);
            _expiration.addMicroSeconds(ms);
        }
    }
}