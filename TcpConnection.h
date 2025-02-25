#pragma once

#include "noncopyable.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"
#include "Timestamp.h"

#include <memory>
#include <string>
#include <atomic>

class Channel;
class EventLoop;
class Socket;

/**
 * TcpServer => Acceptor => 有一个新用户连接，通过accept函数拿到connfd
 * =》 TcpConnection 设置回调 =》 Channel =》 Poller =》 Channel的回调操作
 *
 */
class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop *loop,
                  const std::string &name,
                  int sockfd,
                  const InetAddress &localAddr,
                  const InetAddress &peerAddr);
    ~TcpConnection();

    EventLoop *getLoop() const { return loop_; }
    const std::string &name() const { return name_; }
    const InetAddress &localAddress() const { return localAddr_; }
    const InetAddress &peerAddress() const { return peerAddr_; }

    bool connected() const { return state_ == kConnected; }

    // 发送数据
    void send(const std::string &buf);
    // 关闭连接
    void shutdown();

    void setConnectionCallback(const ConnectionCallback &cb)
    {
        connectionCallback_ = cb;
    }

    void setMessageCallback(const MessageCallback &cb)
    {
        messageCallback_ = cb;
    }

    void setWriteCompleteCallback(const WriteCompleteCallback &cb)
    {
        writeCompleteCallback_ = cb;
    }

    void setHighWaterMarkCallback(const HighWaterMarkCallback &cb, size_t highWaterMark)
    {
        highWaterMarkCallback_ = cb;
        highWaterMark_ = highWaterMark;
    }

    void setCloseCallback(const CloseCallback &cb)
    {
        closeCallback_ = cb;
    }

    // 连接建立
    void connectEstablished();
    // 连接销毁
    void connectDestroyed();

private:
    enum StateE
    {
        kDisconnected,
        kConnecting,
        kConnected,
        kDisconnecting
    }; // 已经断开连接 正在连接 已连接 正在断开连接
    void setState(StateE state) { state_ = state; }

    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void *message, size_t len);
    void shutdownInLoop();

    EventLoop *loop_; // 这里绝对不是baseLoop， 因为TcpConnection都是在subLoop里面管理的
    const std::string name_;
    std::atomic_int state_;
    bool reading_;

    // 这里和Acceptor类似   Acceptor=》mainLoop    TcpConenction=》subLoop
    std::unique_ptr<Socket> socket_; // 连上来的sockfd
    std::unique_ptr<Channel> channel_;

    const InetAddress localAddr_; // 当前主机也就是服务器
    const InetAddress peerAddr_;  // 对端也就是客户端

    ConnectionCallback connectionCallback_;       // 有新连接时的回调
    MessageCallback messageCallback_;             // 有读写消息时的回调
    WriteCompleteCallback writeCompleteCallback_; // 消息发送完成以后的回调
    HighWaterMarkCallback highWaterMarkCallback_; // 不能高于水位线
    CloseCallback closeCallback_;
    size_t highWaterMark_; // 水位线,不能高于它,也就是发送和接收要趋于一致,不能一个过快一个过慢,这样容易出错,发生后就调用这个回调函数调整

    Buffer inputBuffer_;  // 接收数据的缓冲区
    Buffer outputBuffer_; // 发送数据的缓冲区
};