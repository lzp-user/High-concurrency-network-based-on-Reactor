#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb,
                                 const std::string &name)
    : loop_(nullptr), exiting_(false), thread_(std::bind(&EventLoopThread::threadFunc, this), name), mutex_(), cond_(), callback_(cb)
{
}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_ != nullptr)
    {
        loop_->quit();
        thread_.join();
    }
}

EventLoop *EventLoopThread::startLoop()
{
    thread_.start(); // 启动底层的新线程 每次start()都会启动一次新线程

    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (loop_ == nullptr) // 为什么用while不用if,如果被唤醒三个线程,但只有2个消息,那么有一个线程被虚假唤醒,需要while继续等待
        {
            cond_.wait(lock);
        }
        // cond_.wait(lock, [this]{return loop_ != nullptr};); 等价于上面
        loop = loop_;
    }
    return loop;
}

// 下面这个方法，实在单独的新线程里面运行的
void EventLoopThread::threadFunc()
{
    EventLoop loop; // 创建一个独立的eventloop，和上面的线程是一一对应的，one loop per thread

    if (callback_)
    {
        callback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.loop(); // EventLoop loop  => Poller.poll
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}