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
        void addTimerInLoop(Timer* timer);
        bool insert(Timer* timer);
        void resetTimerFd(Timestamp when) const;

        int timerFd;
        std::unique_ptr<Channel> timerChannel;
        EventLoop* ownerLoop;
        typedef std::pair<Timestamp, std::unique_ptr<Timer>> Entry;
        typedef std::set<Entry> TimerList;
        TimerList timers;
        typedef std::unordered_map<Timer::TimerId, Timer*> ActiveTimerList;
        ActiveTimerList activeTimers;
        std::set<Timer::TimerId> cancellingTimers;
    };
}

#endif //EV_TIMERQUEUE_H
