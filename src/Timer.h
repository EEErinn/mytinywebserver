#pragma once
#include <deque>
#include <memory>
#include <queue>
#include <mutex>

#include "TcpConnection.h"
#include "Timestamp.h"
#include "channel.h"
#include "eventloop.h"


namespace mytinywebserver {
// 当Entry引用计数为0时，调用析构函数。Entry添加到TimerManager中。
// 当TimerManager没有Entry，则Entry析构。当TimerManager有Entry，则不析构。
// 析构时，判断Tcpconnection存不存在
class Entry {
   public:
    using WeakTcpConnectionPtr = std::weak_ptr<TcpConnection>;
    explicit Entry(const WeakTcpConnectionPtr &weakConn);
    ~Entry();

   private:
    WeakTcpConnectionPtr m_weakConn;
};

class TimerNode {
   public:
    using TimerPtr = std::shared_ptr<TimerNode>;
    TimerNode(std::shared_ptr<Entry> conn, int timeout);
    ~TimerNode() {}

    Timestamp getExpTime() const { return m_expire; }

   private:
    Timestamp m_expire;  // 超时时间
    std::shared_ptr<Entry> m_conn;
};

struct TimerCmp {
    bool operator()(const TimerNode::TimerPtr &a,
                    const TimerNode::TimerPtr &b) const {
        return b->getExpTime() < a->getExpTime();
    }
};

class TimerManager {
   public:
    TimerManager(EventLoop *loop);
    ~TimerManager();
    void setTimePer(int fd);                     // 设置时钟频率
    void addTimer(TimerNode::TimerPtr);  // 添加定时器
    void handleRead();                           // 超时触发

   private:
    EventLoop *m_loop;
    const int m_timerFd;  // 定时器，相当于alarm
    Channel m_timerChannel;
    std::mutex m_mutex;

    using Deque = std::deque<TimerNode::TimerPtr>;
    std::priority_queue<TimerNode::TimerPtr, Deque, TimerCmp> m_queue;
};
}  // namespace mytinywebserver