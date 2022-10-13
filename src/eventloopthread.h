#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

#include "eventloop.h"

// 避免僵尸进程
namespace mytinywebserver {
class EventLoopThread {
   public:
    EventLoopThread();
    ~EventLoopThread();

    EventLoop* start();
   private:
    void threadFunc();

    EventLoop* m_loop;

    std::thread m_thread;
    std::mutex m_mutex;
    std::condition_variable condition;
};
}  // namespace mytinywebserver