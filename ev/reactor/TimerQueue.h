//
// Created by zr on 23-2-14.
//

#ifndef EV_TIMERQUEUE_H
#define EV_TIMERQUEUE_H
#include "Channel.h"
#include "Timer.h"
#include <set>
#include <unordered_map>
#include <utility>

namespace ev::reactor
{
    class EventLoop;

    class TimerQueue
    {
    public:
        explicit TimerQueue(EventLoop* loop);
        Timer::TimerId addTimer(Timestamp when, double interval, Timer::TimerTask task);
        void cancelTimer(Timer::TimerId);

    private:
        void handleRead();

        int timerFd;
        std::unique_ptr<Channel> timerChannel;
        EventLoop* ownerLoop;
        typedef std::pair<Timestamp, std::unique_ptr<Timer>> Entry;
        typedef std::set<Entry> TimerList;
        TimerList timers;
        typedef std::unordered_map<Timer::TimerId, Timer*> ActiveTimerList;
        ActiveTimerList activeTimerList;
        std::set<Timer::TimerId> cancellingTimers;
    };
}

#endif //EV_TIMERQUEUE_H
