//
// Created by zr on 23-2-14.
//

#include "TimerQueue.h"
#include <sys/timerfd.h>

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

    }

    void TimerQueue::cancelTimer(Timer::TimerId)
    {

    }

    void TimerQueue::handleRead()
    {

    }
}