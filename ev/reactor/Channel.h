//
// Created by zr on 23-2-9.
//

#ifndef EV_CHANNEL_H
#define EV_CHANNEL_H
#include "EventLoop.h"

namespace ev::reactor
{
    class Channel
    {
    public:
        Channel(EventLoop* loop, int fd);
        ~Channel();
    };
}

#endif //EV_CHANNEL_H
