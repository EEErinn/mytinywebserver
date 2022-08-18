#include "Tcpserver.h"

#include <functional>
#include <iostream>
#include <memory>

#include "eventloop.h"
#include "log/LogUtils.h"

namespace mytinywebserver {

const int TcpServer::TIME_OUT = 10;
TcpServer::TcpServer(EventLoop* loop, const InetAddress& addr, int threadNum,
                     const std::string& name)
    : m_loop(loop),
      m_eventloopthreadpool(new EventLoopThreadPool(loop, threadNum)),
      m_acceptor(new Acceptor(loop, addr)),
      m_start(false),
      m_name(name),
      m_ipPort(addr.toString()),
      m_nextConnId(0),
      m_timerQueue(std::make_unique<TimerManager>(loop)) {
    m_acceptor->setNewconnectionCallBack(
        std::bind(&TcpServer::newConnection, this, std::placeholders::_1));
}

TcpServer::~TcpServer() {
    m_loop->assertInThread();
    LOG_DEBUG << "TcpServer::~TcpServer [" << m_name << "] destructing";
    for (auto& item : m_connections) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();  // ?
        conn->getLoop()->runInLoop(
            std::bind(&TcpConnection::connectDestoryed, conn));
    }
}

void TcpServer::start() {
    if (m_start == false) {  // 防止一个tcpserver被start多次
        m_start = true;
        m_eventloopthreadpool->start();
        //开启监听 可能在其他线程调用start 使用runInLoop让listen在创建的线程执行
        m_loop->runInLoop(std::bind(&Acceptor::listen, m_acceptor));
    }
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
    m_loop->runInLoop(
        std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
    m_loop->assertInThread();
    LOG_INFO << "TcpServer::removeConnectionInLoop [" << m_name
             << "] - connection " << conn->getName();
    // 从连接集合移除，tcpconnection引用计数-1
    size_t n = m_connections.erase(conn->getName());
    assert(n == 1);
    EventLoop* connLoop = conn->getLoop();
    // connectDestoryed将channel从监听集合移除，引用计数-1，tcpconnection将析构
    // 一定要用queueInLoop，因为channel在handleEvent调用removeConnectionInLoop，在这里面不能删除channel
    // FIXME: 实验验证不用queueInLoop
    connLoop->queueInLoop(std::bind(&TcpConnection::connectDestoryed, conn));
}

void TcpServer::newConnection(int fd) {
    m_loop->assertInThread();
    char buf[32];
    snprintf(buf, sizeof(buf), "-%s#%d", m_ipPort.c_str(), m_nextConnId++);
    std::string connName = m_name + buf;
    EventLoop* loop = m_eventloopthreadpool->getNextLoop();
    LOG_INFO << "TcpServer::newConnection [" << m_name << "] - new connection ["
             << connName;  //  << "] from " << peerAddr.toIpPort();
    TcpConnectionPtr conn = std::make_shared<TcpConnection>(loop, fd, connName);
    m_connections[connName] = conn;
    conn->setConnectionCallBack(m_connectionCallBack);
    conn->setMessageCallBack(m_messageCallBack);
    conn->setCloseCallBack(
        std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    // 建立连接，channel注册到poller，可读
    loop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
    loop->runInLoop(std::bind(&TcpConnection::newConnTimer, conn,
                              m_timerQueue.get(), TIME_OUT));
}

}  // namespace mytinywebserver