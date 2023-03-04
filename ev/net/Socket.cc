//
// Created by zr on 23-3-2.
//

#include "Socket.h"
#include <strings.h>
#include <unistd.h>
#include <netinet/tcp.h>

namespace ev::net
{
    Socket::Socket(sa_family_t family)
    {
        _fd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
        if(_fd < 0)
            abort();
    }

    Socket::Socket(int sockFd): _fd(sockFd) {}

    Socket::Socket(Socket&& socket) noexcept
    {
        _fd = socket.fd();
        socket._fd = -1;
    }

    Socket& Socket::operator=(Socket&& socket) noexcept
    {
        _fd = socket.fd();
        socket._fd = -1;
        return *this;
    }

    Socket::~Socket()
    {
        if(_fd >= 0)
            ::close(_fd);
    }

    void Socket::setReuseAddr(bool enable) const
    {
        int opt = enable ? 1 : 0;
        if(::setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR,
                        &opt, static_cast<socklen_t>(sizeof(opt))) < 0)
        {
            // TODO handle error
        }
    }

    void Socket::setReusePort(bool enable) const
    {
        int opt = enable ? 1 : 0;
        if(::setsockopt(_fd, SOL_SOCKET, SO_REUSEPORT,
                        &opt, static_cast<socklen_t>(sizeof(opt))) < 0)
        {
            // TODO handle error
        }
    }

    void Socket::setTcpNoDelay(bool enable) const
    {
        int opt = enable ? 1 : 0;
        if(::setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY,
                        &opt, static_cast<socklen_t>(sizeof(opt))) < 0)
        {
            // TODO handle error
        }
    }

    void Socket::setKeepAlive(bool enable) const
    {
        int opt = enable ? 1 : 0;
        if(::setsockopt(_fd, SOL_SOCKET, SO_KEEPALIVE,
                        &opt, static_cast<socklen_t>(sizeof(opt))) < 0)
        {
            // TODO handle error
        }
    }

    void Socket::bind(const Inet4Address& addr) const
    {
        if(::bind(_fd, addr.getSockAddr(), static_cast<socklen_t>(sizeof(struct sockaddr))) < 0)
            abort();
    }

    void Socket::listen() const
    {
        if(::listen(_fd, SOMAXCONN) < 0)
            abort();
    }

    Socket Socket::accept(Inet4Address &address) const
    {
        struct sockaddr addr{};
        bzero(&addr, sizeof(addr));
        socklen_t len = static_cast<socklen_t>(sizeof(addr));
        int fd = ::accept4(_fd, &addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if(fd < 0)
        {
            int savedErrno = errno;
            switch (savedErrno)
            {
                case EAGAIN:
                case ECONNABORTED:
                case EINTR:
                case EPROTO:
                case EPERM:
                case EMFILE:
                    errno = savedErrno;
                    break;
                default: abort();
            }
        }
        else
            address.setSockAddr(&addr);
        return Socket(fd);
    }

    int Socket::connect(const Inet4Address& serverAddr) const
    {
        return ::connect(_fd, serverAddr.getSockAddr(),
                         static_cast<socklen_t>(sizeof(struct sockaddr)));
    }

    int Socket::fd() const {return _fd;}

    void Socket::shutdownWrite() const
    {
        if(::shutdown(_fd, SHUT_WR) < 0)
        {
            // TODO handle error
        }
    }
}