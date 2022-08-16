#pragma once

#include <memory>
#include <vector>

#include "eventloop.h"
#include "eventloopthread.h"

namespace mytinywebserver {

class EventLoopThreadPool {
   public:
    EventLoopThreadPool(EventLoop* loop, int threadNum);
    ~EventLoopThreadPool();

    void setThreadNum(int v) { m_threadNum = v; }

    void start();

    EventLoop* getNextLoop();

   private:
    using EventLoopThreadPtr = std::shared_ptr<EventLoopThread>;
    EventLoop* m_baseLoop;  // 单线程用
    bool m_start;
    int m_threadNum;  // I/O线程数量
    std::vector<EventLoopThreadPtr> m_threads;
    std::vector<EventLoop*> m_loops;
    int nextId;  // robin分配loop
};

}  // namespace mytinywebserver