//
// Created by zr on 23-3-4.
//

#include "TcpConnection.h"

#include <utility>
#include "reactor/EventLoop.h"
#include "reactor/Channel.h"

namespace ev::net
{
    const size_t TcpConnection::DefaultHighWaterMark = 64 * 1024 * 1024; // 64MB 高水位标志

    TcpConnection::TcpConnection(reactor::EventLoop *loop, std::string name, Socket &&socket,
                                 const ev::net::Inet4Address &localAddr,
                                 const ev::net::Inet4Address &peerAddr):
        loop_(loop),
        name_(std::move(name)),
        state_(Connecting),
        socket_(std::move(socket)),
        channel_(new reactor::Channel(loop, socket_.fd())),
        localAddr_(localAddr),
        peerAddr_(peerAddr),
        highWaterMark_(DefaultHighWaterMark)
    {
        channel_->setReadCallback([this] (Timestamp receiveTime) {this->handleRead(receiveTime);});
        channel_->setWriteCallback([this] () {this->handleWrite();});
        channel_->setCloseCallback([this] () {this->handleClose();});
        channel_->setErrorCallback([this] () {this->handleError();});
        socket_.setKeepAlive(true);
    }

    void TcpConnection::connectEstablished()
    {
        loop_->assertInLoopThread();
        assert(state_ == Connecting);
        setState(Connected);
        channel_->tie(shared_from_this());
        if(connectionCallback_)
            connectionCallback_(shared_from_this());
        channel_->enableReading();
    }

    void TcpConnection::startRead() {loop_->runInLoop([this] () {this->startReadInLoop();});}

    void TcpConnection::stopRead() {loop_->runInLoop([this] () {this->stopReadInLoop();});}

    void TcpConnection::startReadInLoop()
    {
        loop_->assertInLoopThread();
        if(!channel_->isReading())
            channel_->enableReading();
    }

    void TcpConnection::stopReadInLoop()
    {
        loop_->assertInLoopThread();
        if(channel_->isReading())
            channel_->disableReading();
    }

    void TcpConnection::shutdown()
    {
        if(state_ == Connected)
        {
            setState(Disconnecting);
            loop_->runInLoop([guard = shared_from_this()] {
                guard->shutdownInLoop();
            });
        }
    }

    void TcpConnection::shutdownInLoop()
    {
        loop_->assertInLoopThread();
        assert(state_ == Disconnecting);
        if(!channel_->isWriting()) //channel_还在关注写事件，说明缓冲区数据还没发送完，不能关闭
            socket_.shutdownWrite();
    }

    void TcpConnection::handleRead(Timestamp receiveTime)
    {
        loop_->assertInLoopThread();
        int savedErrno = 0;
        ssize_t n = inputBuffer_.readFd(socket_.fd(), &savedErrno);
        if (n > 0)
            messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
        // read返回0, 执行关闭的唯一条件
        else if (n == 0)
            handleClose();
        else
        {
            errno = savedErrno;
            handleError();
        }
    }

    void TcpConnection::handleWrite()
    {
        loop_->assertInLoopThread();
        if (channel_->isWriting())
        {
            ssize_t n = socket_.write(outputBuffer_.peek(), outputBuffer_.readableBytes());
            if (n > 0)
            {
                outputBuffer_.retrieve(n);
                if (outputBuffer_.readableBytes() == 0)
                {
                    channel_->disableWriting();
                    if (writeCompleteCallback_)
                        loop_->queueInLoop([guard = shared_from_this()] () {
                            guard->writeCompleteCallback_(guard);
                        });
                    //之前调用过shutdown()但因为数据没发送完而没有执行，现在要再次shutdown
                    if (state_ == Disconnecting)
                        shutdownInLoop();
                }
            }
            else
            {
                // TODO handle error
            }
        }
    }

    void TcpConnection::handleClose()
    {
        loop_->assertInLoopThread();
        assert(state_ == Connected || state_ == Disconnecting);
        setState(Disconnected);
        channel_->disableAll();
        channel_->remove();

        TcpConnectionPtr guardThis(shared_from_this());
        connectionCallback_(guardThis);
        closeCallback_(guardThis);
    }

    void TcpConnection::handleError()
    {
        int err = socket_.error();
        // TODO handle error
    }

    void TcpConnection::setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }

    void TcpConnection::setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }

    void TcpConnection::setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { writeCompleteCallback_ = cb; }

    void TcpConnection::setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
    { highWaterMarkCallback_ = cb; highWaterMark_ = highWaterMark; }

    void TcpConnection::setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }

    reactor::EventLoop* TcpConnection::getLoop() const { return loop_; }

    const std::string& TcpConnection::name() const { return name_; }

    const Inet4Address& TcpConnection::localAddress() const { return localAddr_; }

    const Inet4Address& TcpConnection::peerAddress() const { return peerAddr_; }

    bool TcpConnection::connected() const { return state_ == Connected; }

    bool TcpConnection::disconnected() const { return state_ == Disconnected; }

    void  TcpConnection::setState(StateE s) { state_ = s; }

    void TcpConnection::setTcpNoDelay(bool enable) { socket_.setTcpNoDelay(enable); }
}