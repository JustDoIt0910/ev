//
// Created by zr on 23-2-9.
//

#ifndef EV_EVENTLOOP_H
#define EV_EVENTLOOP_H
#include <functional>
#include <memory>
#include <vector>

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
        void wakeup();
        void runInLoop(Functor f);
        void queueInLoop(Functor f);
        void assertInLoopThread() const;
        [[nodiscard]] bool isInLoopThread() const;

    private:
        std::unique_ptr<EPoller> poller;
        int wakeupFd;
        std::unique_ptr<Channel> wakeupChannel;
        std::vector<Functor> taskQueue;
        const pid_t loopThread;
    };
}

#endif //EV_EVENTLOOP_H
