#pragma once

#include <atomic>
#include <map>
#include <string>

#include "Acceptor.h"
#include "CallBacks.h"
#include "TcpConnection.h"
#include "Timer.h"
#include "eventloopthreadpool.h"

namespace mytinywebserver {

/**
 * @brief
 * 用户直接使用，生命周期由用户控制。用户只需设置号callback，再调用start。
 * 拥有 listen sock
 * 拥有 eventloopThreadPool
 */
class TcpServer {
   public:
    static const int TIME_OUT;  // 超时时间 10s
    TcpServer(EventLoop* loop, const InetAddress& addr, int threadNum,
              const std::string& name);
    ~TcpServer();

    // 开启事件循环子线程和事件监听
    void start();

    // 设置用户层业务逻辑函数
    void setMessageCallBack(MessageCallBack_ v) { m_messageCallBack = v; }
    void setConnectionCallBack(ConnectionCallBack_ v) {
        m_connectionCallBack = v;
    }

    const std::string& getName() const { return m_name; }
    const std::string& getIpPort() const { return m_ipPort; }
    // FIXMEE
    const std::unique_ptr<TimerManager>& getTimerManager() const {
        return m_timerQueue;
    }

   private:
    void newConnection(int fd);  // 建立新连接

    // 销毁连接
    void removeConnection(const TcpConnectionPtr&);
    void removeConnectionInLoop(const TcpConnectionPtr&);

    EventLoop* m_loop;  // 用户定义的loop，主线程的mainloop
    std::unique_ptr<EventLoopThreadPool> m_eventloopthreadpool;
    Acceptor* m_acceptor;       // 管理listen sock
    std::atomic<bool> m_start;  // 是否启动

    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;
    ConnectionMap m_connections;  // 连接集合
    const std::string m_name;     // tcpserver的名称
    const std::string m_ipPort;   // server绑定的ip和port
    int m_nextConnId;             // 为了标记创建的连接

    // 业务逻辑函数
    ConnectionCallBack_ m_connectionCallBack;
    MessageCallBack_ m_messageCallBack;

    std::unique_ptr<TimerManager> m_timerQueue;  // 定时器
};

}  // namespace mytinywebserver