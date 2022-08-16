#include "eventloopthreadpool.h"

#include <assert.h>

#include "log/LogUtils.h"

namespace mytinywebserver {
EventLoopThreadPool::EventLoopThreadPool(EventLoop* loop, int threadNum)
    : m_baseLoop(loop), m_start(false), m_threadNum(threadNum), nextId(0) {}

EventLoopThreadPool::~EventLoopThreadPool() {
    LOG_DEBUG << "EventLoopThreadPool::~EventLoopThreadPool() destructing";
}

void EventLoopThreadPool::start() {
    m_baseLoop->assertInThread();
    m_start = true;
    for (int i = 0; i < m_threadNum; ++i) {
        EventLoopThreadPtr t(new EventLoopThread());
        m_threads.push_back(t);
        m_loops.push_back(t->start());
    }
}

EventLoop* EventLoopThreadPool::getNextLoop() {
    m_baseLoop->assertInThread();
    assert(m_start);
    EventLoop* loop = m_baseLoop;
    if (!m_loops.empty()) {
        loop = m_loops[nextId];
        nextId = (nextId + 1) % m_threadNum;
    }
    return loop;
}
}  // namespace mytinywebserver