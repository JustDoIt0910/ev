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

namespace ev {class Timestamp;}

namespace ev::reactor
{
    class Channel;
    class EPoller;

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
        void assertInLoopThread() const;
        [[nodiscard]] bool isInLoopThread() const;
        [[nodiscard]] bool hasChannel(Channel* channel) const;

    private:
        void handleRead(Timestamp receiveTime) const;
        void doPendingFunctors();

        std::unique_ptr<EPoller> poller;
        int wakeupFd;
        std::unique_ptr<Channel> wakeupChannel;
        std::mutex mu; //保护taskQueue
        typedef std::vector<Functor> TaskQueue;
        TaskQueue taskQueue;
        const pid_t loopThread;
        std::atomic_bool running;
        bool eventHandling;
        bool callingPendingFunctors;

        typedef std::vector<Channel*> ChannelList;
        ChannelList activeChannels;
        Channel* currentActiveChannel;
    };
}

#endif //EV_EVENTLOOP_H
