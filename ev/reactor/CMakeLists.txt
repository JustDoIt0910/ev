set(REACTOR_SRC EventLoop.cc
        Channel.cc
        EPoller.cc
        utils/CurrentThread.cc)

add_library(reactor ${REACTOR_SRC})