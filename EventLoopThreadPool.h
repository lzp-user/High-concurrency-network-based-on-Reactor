#pragma once
#include "noncopyable.h"

#include <functional>
#include <string>
#include <vector>
#include <memory>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    EventLoopThreadPool(EventLoop *baseLoop, const std::string &nameArg);
    ~EventLoopThreadPool(); // 不需要析构eventloop,因为底层eventloop不是创建的指针

    void setThreadNum(int numThreads) { numThreads_ = numThreads; } // 设置线程数量

    void start(const ThreadInitCallback &cb = ThreadInitCallback());

    // 如果工作在多线程中，baseLoop_默认以轮询的方式分配channel给subloop
    EventLoop *getNextLoop();

    std::vector<EventLoop *> getAllLoops();

    bool started() const { return started_; }
    const std::string name() const { return name_; }

private:
    EventLoop *baseLoop_; // EventLoop loop; mainloop 用户创建的
    std::string name_;
    bool started_;
    int numThreads_;
    int next_;                                              // 用于做轮询的下标
    std::vector<std::unique_ptr<EventLoopThread>> threads_; // 所有事件的线程
    std::vector<EventLoop *> loops_;                        // 所有事件
};