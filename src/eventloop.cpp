// Copyright©2022 Erin
#include "eventloop.h"

#include <assert.h>
#include <sys/eventfd.h>

#include <iostream>

#include "CurrentThread.h"
#include "channel.h"
#include "epollpoller.h"
#include "log/LogManager.h"
#include "socketops.h"

namespace mytinywebserver {

int createEventFd() {
    int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (fd < 0) {
        LOG_ERROR << "Failed in eventfd";
    }
    return fd;
}

// 用__thread关键字修饰的全局变量，每个线程都有一个eventloop实体，但互不干扰。相当于所有线程都有这样一个局部变量。
__thread EventLoop* t_loopInThisThread = nullptr;

EventLoop::EventLoop()
    : m_poller(new EpollPoller()),
      m_threadId(CurrentThread::tid()),
      m_looping(false),
      m_quit(false),
      m_callingPendingFunctors(false),
      m_wakeupfd(createEventFd()),
      m_wakeupChannel(new Channel(this, m_wakeupfd)) {
    LOG_DEBUG << "EventLoop created " << this << " in thread " << m_threadId;
    if (t_loopInThisThread) {
        LOG_FATAL << "Another Eventloop " << t_loopInThisThread
                  << "exists in this thread " << m_threadId;
    } else {
        t_loopInThisThread = this;
    }
    m_wakeupChannel->setReadCallBack_(
        std::bind(&EventLoop::handleRead, this, std::placeholders::_1));
    m_wakeupChannel->enableReading();
}

EventLoop::~EventLoop() {
    assert(!m_looping);
    LOG_DEBUG << "EventLoop " << this << " of thread " << m_threadId
              << " destructs in thread " << CurrentThread::tid();
    m_wakeupChannel->disableAll();
    m_wakeupChannel->removeSelf();
    socket::closeFd(m_wakeupfd);
    t_loopInThisThread = nullptr;
}

bool EventLoop::isInloopThread() const {
    return m_threadId == CurrentThread::tid();
}

void EventLoop::assertInThread() {
    if (!isInloopThread()) {
        LOG_DEBUG << "EventLoop::abortNotInLoopThread - EventLoop " << this
                  << " was created in threadId_ = " << m_threadId
                  << ", current thread id = " << CurrentThread::tid();
    }
}

void EventLoop::loop() {
    assert(!m_looping);
    assertInThread();
    m_looping = true;
    m_quit = false;
    LOG_DEBUG << "EventLoop " << this << " start looping";
    while (!m_quit) {
        m_activeChannels.clear();
        m_poller->poll(&m_activeChannels);

        for (auto* channel : m_activeChannels) {
            channel->handleEvent(Timestamp::now());
        }
        doPendingFunctors();  //处理其他任务，执行外部线程注入的函数对象
    }
    LOG_DEBUG << "Eventloop " << this << " stop looping";
    m_looping = false;
}

/**
 * 退出事件循环
 * 1. 如果loop在自己的线程中调用quit成功了
 *    说明当前线程已经执行完毕了loop()函数的poller_->poll并退出
 * 2. 如果不是当前EventLoop所属线程中调用quit退出EventLoop
 *    需要唤醒EventLoop所属线程的epoll_wait
 *
 *    比如在一个subloop(worker)中调用mainloop(IO)的quit时
 *    需要唤醒mainloop(IO)的poller_->poll 让其执行完loop()函数
 *
 *    ！！！ 注意： 正常情况下 mainloop负责请求连接 将回调写入subloop中
 *    通过生产者消费者模型即可实现线程安全的队列 ！！！
 *    但是muduo通过wakeup()机制 使用eventfd创建的wakeupFd_ notify
 *    使得mainloop和subloop之间能够进行通信
 **/
void EventLoop::quit() {
    m_quit = true;
    if (!isInloopThread()) {
        wakeup();
    }
}

// 在所属线程则直接执行，在其他线程则放入等待队列，唤醒所属线程来执行该函数
void EventLoop::runInLoop(Functor cb) {
    if (isInloopThread()) {
        cb();
    } else {
        queueInLoop(cb);
    }
}
void EventLoop::queueInLoop(Functor cb) {
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_pendingFunctors.push_back(cb);
    }
    /**
     * || callingPendingFunctors的意思是 当前loop正在执行回调中
     * 但是loop的pendingFunctors_中又加入了新的回调 需要通过wakeup写事件
     * 唤醒相应的需要执行上面回调操作的loop的线程
     * 让loop()下一次poller_->poll()不再阻塞（阻塞的话会延迟前一次新加入的回调的执行），然后
     * 继续执行pendingFunctors_中的回调函数
     **/
    if (!isInloopThread() || m_callingPendingFunctors) {
        wakeup();
    }
}

void EventLoop::doPendingFunctors() {
    assertInThread();
    m_callingPendingFunctors = true;
    std::vector<Functor> functors;
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        functors.swap(m_pendingFunctors);
    }

    for (const Functor& f : functors) {
        f();
    }
    m_callingPendingFunctors = false;
}

void EventLoop::updateChannel(Channel* channel) {
    assertInThread();
    m_poller->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) {
    assertInThread();
    m_poller->removeChannel(channel);
}
bool EventLoop::hasChannel(Channel* channel) {
    assertInThread();
    return m_poller->hasChannel(channel);
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    auto n = write(m_wakeupfd, &one, sizeof(one));
    LOG_DEBUG << "EventLoop::wakeup()";
    if (n != sizeof(one)) {
        LOG_ERROR << "EventLoop::wakeup() writes " << n
                  << " bytes instead of 8";
    }
}
void EventLoop::handleRead(Timestamp receiveTime) {
    uint64_t one = 1;
    auto n = read(m_wakeupfd, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR << "EventLoop::handleRead() reads " << n
                  << " bytes instead of 8";
    }
}

}  // namespace mytinywebserver
