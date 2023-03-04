//
// Created by zr on 23-3-5.
//
#include "net/Connector.h"
#include "reactor/EventLoop.h"
#include <memory>
#include <iostream>

using namespace ev::reactor;
using namespace ev::net;

int main()
{
    EventLoop loop;
    Socket socket;
    std::shared_ptr<Connector> connector(new Connector(&loop, Inet4Address("127.0.0.1", 9999)));
    connector->setNewConnectionCallback([&socket](Socket s) {
        socket = std::move(s);
        std::cout << "connected" <<std::endl;
    });
    connector->start();
    loop.loop();
    return 0;
}