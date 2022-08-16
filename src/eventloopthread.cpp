#include "eventloopthread.h"

#include "log/LogUtils.h"
namespace mytinywebserver {
EventLoopThread::EventLoopThread() : m_loop(nullptr) {}

EventLoopThread::~EventLoopThread() {
    if (m_loop != nullptr) {
        m_loop->quit();
        m_thread.join();
    }
}

EventLoop* EventLoopThread::start() {
    // 子线程
    m_thread = std::thread(std::bind(&EventLoopThread::threadFunc, this));
    // 主线程
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_loop == nullptr) {
            condition.wait(lock);
        }
    }
    return m_loop;
}

void EventLoopThread::threadFunc() {
    LOG_DEBUG << " eventloopthread start";
    EventLoop loop;
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_loop = &loop;
        condition.notify_one();
    }
    loop.loop();
    m_loop = NULL;
}
}  // namespace mytinywebserver