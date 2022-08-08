#include "Tcpserver.h"

#include <functional>
#include <memory>

#include "eventloop.h"

namespace mytinywebserver {

TcpServer::TcpServer(EventLoop* loop, const InetAddress& addr,
                     const std::string& name)
    : m_loop(loop),
      m_acceptor(new Acceptor(loop, addr)),
      m_name(name),
      m_nextConnId(0) {
    m_acceptor->setNewconnectionCallBack(
        std::bind(&TcpServer::newConnection, this, std::placeholders::_1));
}

TcpServer::~TcpServer() {}

void TcpServer::start() {
    if (m_start == false) {  // 防止一个tcpserver被start多次
        m_start = true;

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
    // 从连接集合移除，tcpconnection引用计数-1
    size_t n = m_connections.erase(conn->getName());
    assert(n == 1);

    // connectDestoryed将channel从监听集合移除，引用计数-1，tcpconnection将析构
    EventLoop* connLoop = conn->getLoop();
    // 一定要用queueInLoop，因为channel在handleEvent调用removeConnectionInLoop，在这里面不能删除channel
    // FIXME: 实验验证不用queueInLoop
    connLoop->queueInLoop(std::bind(&TcpConnection::connectDestoryed, conn));
}

void TcpServer::newConnection(int fd) {
    m_loop->assertInThread();
    /************************/
    char buf[32];
    snprintf(buf, sizeof(buf), "#%d", m_nextConnId++);
    std::string connName = m_name + buf;
    EventLoop loop;  // FIXME: 从线程池中取出一个loop
    /************************/
    TcpConnectionPtr conn =
        std::make_shared<TcpConnection>(&loop, fd, connName);
    m_connections[connName] = conn;
    // 设置tcpconnection
    conn->setConnectionCallBack(m_connectionCallBack);
    conn->setMessageCallBack(m_messageCallBack);
    conn->setCloseCallBack(
        std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    conn->connectEstablished();  // 建立连接，channel注册到poller，可读
}

}  // namespace mytinywebserver