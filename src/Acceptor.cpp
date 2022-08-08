// Copyright©2022 Erin
#include "Acceptor.h"

#include "channel.h"
#include "eventloop.h"
#include "socketops.h"

namespace mytinywebserver {

Acceptor::Acceptor(EventLoop* loop, const InetAddress& addr)
    : m_loop(loop),
      m_listenSock(socket::createSocketFd()),
      m_listening(false),
      m_channel(new Channel(loop, m_listenSock.getSockFd())) {
    m_listenSock.bind(addr);  // bind
    m_channel->setReadCallBack_(
        std::bind(&Acceptor::handleRead, this, std::placeholders::_1));
}

Acceptor::~Acceptor() {
    // m_channel->disableAll();
    m_channel->removeSelf();
}

void Acceptor::listen() {
    m_listening = true;
    m_listenSock.listen();
    m_channel->enableReading();
}

void Acceptor::handleRead(Timestamp receiveTime) {
    int connfd = m_listenSock.accept();  // 返回客户端连接的fd
    if (connfd > 0) {
        if (m_newConnectionCallBack) {
            m_newConnectionCallBack(connfd);
        } else {
            socket::closeFd(connfd);
        }
    }
}

}  // namespace mytinywebserver