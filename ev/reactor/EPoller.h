//
// Created by zr on 23-2-9.
//

#ifndef EV_EPOLLER_H
#define EV_EPOLLER_H
#include "EventLoop.h"

namespace ev::reactor
{
    class EPoller
    {
    public:
        EPoller(EventLoop* loop);
    };
}

#endif //EV_EPOLLER_H
