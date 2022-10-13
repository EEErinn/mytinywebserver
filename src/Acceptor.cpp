// Copyright©2022 Erin
#include "Acceptor.h"

#include "channel.h"
#include "eventloop.h"
#include "log/LogManager.h"
#include "socketops.h"

namespace mytinywebserver {

Acceptor::Acceptor(EventLoop* loop, const InetAddress& addr)
    : m_loop(loop),
      m_listenSock(socket::createSocketFd()),  // 创建listen sock
      m_listening(false),
      m_channel(new Channel(loop, m_listenSock.getSockFd())) {
    m_listenSock.setReUse(true);
    int on = 1;
    ::setsockopt(m_listenSock.getSockFd(), SOL_SOCKET, SO_OOBINLINE, &on,
                 static_cast<socklen_t>(sizeof on));
    m_listenSock.bind(addr);
    m_channel->setReadCallBack_(
        std::bind(&Acceptor::handleRead, this, std::placeholders::_1));
}

Acceptor::~Acceptor() {
    m_channel->disableAll();
    m_channel->removeSelf();
}

void Acceptor::listen() {
    LOG_DEBUG << "Acceptor::listen";
    m_listening = true;
    m_listenSock.listen();
    m_channel->enableReading();  // 注册进poller监测集合
}

void Acceptor::handleRead(Timestamp receiveTime) {
    int connfd = 0;  // 返回客户端连接的fd
    while ((connfd = m_listenSock.accept()) > 0) {
        if (m_newConnectionCallBack) {
            m_newConnectionCallBack(connfd);
        } else {
            socket::closeFd(connfd);
        }
    }
    if (connfd < 0) {
        LOG_ERROR << "in Acceptor::handleRead";
    }
}

}  // namespace mytinywebserver