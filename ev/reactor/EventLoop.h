//
// Created by zr on 23-2-9.
//

#ifndef EV_EVENTLOOP_H
#define EV_EVENTLOOP_H
#include <functional>
#include <memory>
#include <vector>
#include <atomic>
#include <mutex>
#include "Timer.h"

namespace ev {class Timestamp;}

namespace ev::reactor
{
    class Channel;
    class EPoller;
    class TimerQueue;

    class EventLoop
    {
    public:
        typedef std::function<void()> Functor;

        EventLoop();
        ~EventLoop();
        void loop();
        void quit();
        void updateChannel(Channel* channel);
        void removeChannel(Channel* channel);
        void wakeup() const;
        void runInLoop(Functor f);
        void queueInLoop(Functor f);
        Timer::TimerId runAt(Timestamp when, Timer::TimerTask task);
        Timer::TimerId runAfter(double delay, Timer::TimerTask task);
        Timer::TimerId runEvery(double interval, Timer::TimerTask task);
        void cancelTimer(Timer::TimerId id);
        void assertInLoopThread() const;
        [[nodiscard]] bool isInLoopThread() const;
        [[nodiscard]] bool hasChannel(Channel* channel) const;

    private:
        void handleRead(Timestamp receiveTime) const;
        void doPendingFunctors();

        const pid_t loopThread;
        int wakeupFd;
        std::atomic_bool running;
        bool eventHandling;
        bool callingPendingFunctors;
        std::unique_ptr<Channel> wakeupChannel;
        std::unique_ptr<EPoller> poller;
        std::unique_ptr<TimerQueue> timerQueue;
        std::mutex mu; //保护taskQueue
        typedef std::vector<Functor> TaskQueue;
        TaskQueue taskQueue;
        typedef std::vector<Channel*> ChannelList;
        ChannelList activeChannels;
        Channel* currentActiveChannel;
    };
}

#endif //EV_EVENTLOOP_H
