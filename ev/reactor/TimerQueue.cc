//
// Created by zr on 23-2-14.
//

#include "TimerQueue.h"
#include <sys/timerfd.h>
#include <cassert>
#include <strings.h>

namespace ev::reactor
{
    static int createTimerFd()
    {
        int timerFd = ::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
        if(timerFd < 0)
        {
            // TODO handle error
        }
        return timerFd;
    }

    struct timespec howMuchTimeFromNow(Timestamp when)
    {
        Timestamp now = Timestamp::now();
        uint64_t microseconds = when - now;
        struct timespec ts{};
        ts.tv_sec = static_cast<time_t>(
                microseconds / Timestamp::microSecondsPerSecond);
        ts.tv_nsec = static_cast<long>(
                (microseconds % Timestamp::microSecondsPerSecond) * 1000);
        return ts;
    }

    void TimerQueue::resetTimerFd(Timestamp when) const
    {
        struct itimerspec newVal{};
        bzero(&newVal,  sizeof(newVal));
        newVal.it_value = howMuchTimeFromNow(when);
        //flags为0，超时时间被解释为相对时间, 设置了TFD_TIMER_ABSTIME则为绝对时间
        int ret = ::timerfd_settime(timerFd, 0, &newVal, nullptr);
        if(ret < 0)
        {
            // TODO handle error
        }
    }

    TimerQueue::TimerQueue(EventLoop *loop):
        ownerLoop(loop),
        timerFd(createTimerFd()),
        timerChannel(new Channel(loop, timerFd))
    {
        timerChannel->setReadCallback(std::bind(&TimerQueue::handleRead, this));
        timerChannel->enableReading();
    }

    Timer::TimerId TimerQueue::addTimer(Timestamp when, double interval, Timer::TimerTask task)
    {
        Timer* timer = new Timer(std::move(task), when, interval);
        ownerLoop->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
        return timer->id();
    }

    void TimerQueue::addTimerInLoop(Timer* timer)
    {
        ownerLoop->assertInLoopThread();
        bool earliestChanged = insert(timer);
        if(earliestChanged)
            resetTimerFd(timer->expiration());
    }

    bool TimerQueue::insert(Timer *timer)
    {
        assert(timers.size() == activeTimers.size());
        Entry newTimer = std::make_pair(timer->expiration(), std::unique_ptr<Timer>(timer));
        bool earliestChanged = false;
        auto earliest = timers.cbegin();
        /* 之前timers里没有timer或者新timer超时时间早于timers里第一个timer
         * 说明timerFd的超时时间需要调整 */
        if(earliest == timers.cend() || newTimer.first < earliest->first)
            earliestChanged = true;
        {
            auto result = timers.insert(std::move(newTimer));
            assert(result.second);
        }
        {
            std::pair<Timer::TimerId, Timer*> activeTimer = std::make_pair(timer->id(), timer);
            auto result = activeTimers.insert(activeTimer);
            assert(result.second);
        }
        assert(timers.size() == activeTimers.size());
        return earliestChanged;
    }

    void TimerQueue::cancelTimer(Timer::TimerId)
    {

    }

    void TimerQueue::handleRead()
    {

    }
}