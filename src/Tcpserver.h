#pragma once

#include <atomic>
#include <map>
#include <string>

#include "Acceptor.h"
#include "CallBacks.h"
#include "TcpConnection.h"

namespace mytinywebserver {

/**
 * @brief
 * 用户直接使用，生命周期由用户控制。用户只需设置号callback，再调用start。
 * 拥有 listen sock
 * 拥有 eventloopThreadPool
 */
class TcpServer {
   public:
    TcpServer(EventLoop* loop, const InetAddress& addr,
              const std::string& name);
    ~TcpServer();

    // 开启事件循环子线程和事件监听
    void start();

    // 设置用户层业务逻辑函数
    void setMessageCallBack(MessageCallBack_ v) { m_messageCallBack = v; }
    void setConnectionCallBack(ConnectionCallBack_ v) {
        m_connectionCallBack = v;
    }

   private:
    void newConnection(int fd);  // 建立新连接

    // 销毁连接
    void removeConnection(const TcpConnectionPtr&);
    void removeConnectionInLoop(const TcpConnectionPtr&);

    EventLoop* m_loop;          // 用户定义的loop，主线程的mainloop
    Acceptor* m_acceptor;       // 管理listen sock
    std::atomic<bool> m_start;  // 是否启动

    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;
    ConnectionMap m_connections;  // 连接集合
    const std::string m_name;     // tcpserver的名称
    int m_nextConnId;             // 为了标记创建的连接

    // 业务逻辑函数
    ConnectionCallBack_ m_connectionCallBack;
    MessageCallBack_ m_messageCallBack;
};

}  // namespace mytinywebserver