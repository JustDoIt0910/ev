set(REACTOR_SRC Inet4Address.cc
        Socket.cc
        Buffer.cc TcpConnection.cc TcpConnection.h Connector.cc Connector.h)

add_library(net ${REACTOR_SRC})