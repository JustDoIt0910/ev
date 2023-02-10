//
// Created by zr on 23-2-9.
//

#include "EventLoop.h"
#include "EPoller.h"
#include "Channel.h"
#include "../utils/CurrentThread.h"
#include <sys/eventfd.h>

namespace ev::reactor
{

    static int createEventFd()
    {
        int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if(fd < 0)
        {
            // TODO handle error
        }
        return fd;
    }

    EventLoop::EventLoop():
        wakeupFd(createEventFd()),
        wakeupChannel(new Channel(this, wakeupFd)),
        poller(new EPoller(this)),
        loopThread(currentTreadId())
    {

    }

    EventLoop::~EventLoop()
    {

    }

    void EventLoop::loop()
    {

    }

    void EventLoop::quit()
    {

    }

    void EventLoop::updateChannel(Channel* channel)
    {

    }

    void EventLoop::removeChannel(Channel* channel)
    {

    }

    void EventLoop::wakeup()
    {

    }

    void EventLoop::runInLoop(Functor f)
    {

    }

    void EventLoop::queueInLoop(Functor f)
    {

    }

    void EventLoop::assertInLoopThread() const
    {
        if(!isInLoopThread())
            abort();
    }

    bool EventLoop::isInLoopThread() const {return loopThread == currentTreadId();}
}