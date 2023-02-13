//
// Created by zr on 23-2-9.
//

#include "EventLoop.h"
#include "EPoller.h"
#include "Channel.h"
#include "../utils/CurrentThread.h"
#include <sys/eventfd.h>
#include <unistd.h>
#include <functional>
#include <cassert>

using namespace std::placeholders;

namespace ev::reactor
{
    __thread EventLoop* g_LoopInThisThread = nullptr;

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
        loopThread(currentTreadId()),
        currentActiveChannel(nullptr),
        running(false),
        eventHandling(false),
        callingPendingFunctors(false)
    {
        if(!g_LoopInThisThread)
            g_LoopInThisThread = this;
        else
            abort();
        wakeupChannel->setReadCallback(std::bind(&EventLoop::handleRead, this, _1));
        wakeupChannel->enableReading();
    }

    EventLoop::~EventLoop()
    {
        wakeupChannel->disableAll();
        wakeupChannel->remove();
        ::close(wakeupFd);
        g_LoopInThisThread = nullptr;
    }

    void EventLoop::loop()
    {
        assertInLoopThread();
        running = true;
        while (running)
        {
            activeChannels.clear();
            Timestamp eventTime = poller->poll(EPoller::EpollTimeoutMs, activeChannels);
            eventHandling = true;
            for(Channel* channel: activeChannels)
            {
                currentActiveChannel = channel;
                currentActiveChannel->handleEvent(eventTime);
            }
            eventHandling = false;
            doPendingFunctors();
        }
    }

    void EventLoop::quit()
    {
        running = false;
        if(!isInLoopThread())
            wakeup();
    }

    void EventLoop::updateChannel(Channel* channel)
    {
        assert(channel->ownerLoop() == this);
        assertInLoopThread();
        poller->updateChannel(channel);
    }

    void EventLoop::removeChannel(Channel* channel)
    {
        assert(channel->ownerLoop() == this);
        assertInLoopThread();
        if(eventHandling)
        {
            assert(currentActiveChannel != channel ||
                std::find(activeChannels.begin(), activeChannels.end(), channel) == activeChannels.end());
        }
        poller->removeChannel(channel);
    }

    void EventLoop::wakeup() const
    {
        uint64_t one = 1;
        ssize_t n = ::write(wakeupFd, &one, sizeof(one));
        if(n != sizeof(one))
        {
            // TODO handle error
        }
    }

    void EventLoop::runInLoop(Functor f)
    {
        if(isInLoopThread())
            f();
        else
            queueInLoop(std::move(f));
    }

    void EventLoop::queueInLoop(Functor f)
    {
        {
            std::lock_guard<std::mutex> lg(mu);
            taskQueue.push_back(std::move(f));
        }
        /* 为了能使得新加入的任务及时被执行，有两种情况需要唤醒poller
         * 1. queueInLoop的调用线程不是本线程，这时loop线程在做什么是不一定的，可能正阻塞在epoll_waite上，
         * 也可能正在调用pending functors，都会导致新task不能立即被执行
         * 2. loop线程自己调用queueInLoop，有两种可能的时间点，eventHandling期间或callingPendingFunctor期间，
         * eventHandling期间添加新functor可以被立刻执行到，而callingPendingFunctor期间新添加的任务需要下一个循环才会被
         * 执行，需要唤醒，避免阻塞在poll()上*/
        if(!isInLoopThread() || callingPendingFunctors)
            wakeup();
    }

    void EventLoop::assertInLoopThread() const
    {
        if(!isInLoopThread())
            abort();
    }

    bool EventLoop::isInLoopThread() const {return loopThread == currentTreadId();}

    bool EventLoop::hasChannel(Channel* channel) const
    {
        assertInLoopThread();
        return poller->hasChannel(channel);
    }

    void EventLoop::handleRead(Timestamp receiveTime) const
    {
        uint64_t one;
        ssize_t n = ::read(wakeupFd, &one, sizeof(one));
        if(n != sizeof(one))
        {
            // TODO handle error
        }
    }

    void EventLoop::doPendingFunctors()
    {
        assertInLoopThread();
        callingPendingFunctors = true;
        TaskQueue functors;
        {
            std::lock_guard<std::mutex> lg(mu);
            functors.swap(taskQueue);
        }
        for(const auto& f: functors)
            f();
        callingPendingFunctors = false;
    }
}