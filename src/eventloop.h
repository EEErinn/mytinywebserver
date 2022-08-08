// Copyright©2022 Erin
#pragma once

#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "Timestamp.h"
#include "noncopyable.h"

namespace {

class IgnoreSigPipe {
   public:
    IgnoreSigPipe() { ::signal(SIGPIPE, SIG_IGN); }
};
IgnoreSigPipe initObj;
}  // namespace
namespace mytinywebserver {

class Channel;
class Poller;

// eventloop 拥有poller
// one loop per thread 保证线程安全
// 线程需要I/O处理则使用自己的eventloop，防止竞态条件
class EventLoop : noncopyable {
   public:
    EventLoop();
    ~EventLoop();

    bool isInloopThread() const;  // 当前线程是否是创建eventloop对象的线程
    void assertInThread();  // 当前线程是创建eventloop对象的线程，否则报错

    // 必须在I/O线程执行
    void loop();  // 开启事件循环
    void quit();  //退出事件循环

    // 可以被其他线程调用，注意加锁保证线程安全
    using Functor = std::function<void()>;
    void runInLoop(Functor cb);
    void queueInLoop(Functor cb);
    void doPendingFunctors();

    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    void wakeup();  // 用来唤醒loop所在线程
                    // 往wakeupfd中写入数据，loop的epoll_wait返回
    // 往channel中注册在wakeup上发生读事件时的回调函数
    void handleRead(Timestamp receiveTime);

    int getThreadId() { return m_threadId; }

   private:
    // 返回Poller监测到有事件发生的channel列表，用于缓存
    using ChannelList = std::vector<Channel*>;
    ChannelList m_activeChannels;

    std::unique_ptr<Poller> m_poller;  //

    const pid_t m_threadId;  // 创建该对象的线程Id

    std::atomic<bool> m_looping;  // 是否正在循环
    std::atomic<bool> m_quit;     // 是否要退出循环/是否在循环外

    std::mutex m_mutex;
    std::vector<Functor> m_pendingFunctors;  //
    std::atomic<bool> m_callingPendingFunctors;

    int m_wakeupfd;  // 为唤醒处于epoll_wait中的eventloop对象
    std::unique_ptr<Channel> m_wakeupChannel;  // wakeupfd对应的channel
};

}  // namespace mytinywebserver