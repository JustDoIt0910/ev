//
// Created by zr on 23-3-2.
//

#ifndef EV_SOCKET_H
#define EV_SOCKET_H
#include "Inet4Address.h"
#include "utils/noncopyable.h"

namespace ev::net
{
    class Socket : public noncopyable
    {
        explicit Socket(sa_family_t family);
        explicit Socket(int sockFd);
        Socket(Socket&& socket) noexcept;
        Socket& operator=(Socket&& socket) noexcept;
        ~Socket();
        void setReuseAddr(bool enable) const;
        void setReusePort(bool enable) const;
        void setTcpNoDelay(bool enable) const;
        void setKeepAlive(bool enable) const;
        void bind(const Inet4Address& addr) const;
        void listen() const;
        Socket accept(Inet4Address& addr) const;
        void shutdownWrite() const;
        [[nodiscard]] int connect(const Inet4Address& serverAddr) const;
        [[nodiscard]] int fd() const;

    private:
        int _fd;
    };
}

#endif //EV_SOCKET_H
