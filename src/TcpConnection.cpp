#include "TcpConnection.h"

#include <unistd.h>

namespace mytinywebserver {

TcpConnection::TcpConnection(EventLoop* loop, int fd, const std::string& name)
    : m_loop(loop),
      m_name(name),
      m_state(kconnecting),
      m_connSocket(new Socket(fd)),
      m_channel(new Channel(loop, fd)) {
    m_channel->setReadCallBack_(
        std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    m_channel->setWriteCallBack_(std::bind(&TcpConnection::handleWrite, this));
    m_channel->setErrorCallBack_(std::bind(&TcpConnection::handleError, this));
    m_channel->setCloseCallBack_(std::bind(&TcpConnection::handleClose, this));
}

TcpConnection::~TcpConnection() {
    // LOG_DEBUG << "TcpConnection::dtor[" << name_ << "] at " << this
    //           << " fd=" << channel_->fd() << " state=" << stateToString();
    assert(m_state == kDisconnected);
}

void TcpConnection::connectEstablished() {
    assert(m_state == StateE::kconnecting);
    setState(StateE::kConnected);
    m_channel->tie(shared_from_this());
    m_channel->enableReading();
    m_connectionCallBack(shared_from_this());
}

void TcpConnection::connectDestoryed() {
    m_loop->assertInThread();
    if (m_state == StateE::kConnected) {
        setState(StateE::kDisconnected);
        m_channel->disableAll();
        // FIXME: 为什么是m_connectionCallBack而不是m_closeCallBack
        m_connectionCallBack(shared_from_this());
    }
    m_channel->removeSelf();
}

void TcpConnection::send(const void* message, size_t len) {
    // 将message写入outputBuffer
}
void TcpConnection::send(const std::string& message) {
    if (m_state == kConnected) {
        m_loop->runInLoop(std::bind(&TcpConnection::sendInloop, this,
                                    message.data(), message.size()));
    }
}
void TcpConnection::sendInloop(const void* message, size_t len) {
    m_loop->assertInThread();
    // 先直接发送，若没有发送完，则存在output中，开启写事件
    size_t n = 0;
    bool error = false;
    if (!m_channel->isWritable() && m_outputBuffer.readableSize() == 0) {
        n = ::send(m_channel->getFd(), message, len, 0);
        if (n >= 0) {
            if (n == len) {  //写完了，应该执行什么？
                // shutdown(); // ??
            }
        } else {
            if (errno != EAGAIN || errno != EWOULDBLOCK) {
                if (errno == EPIPE || errno == ECONNRESET) {
                    //  忽略错误，让epoll处理？
                    error = true;
                }
            }
        }
    }
    // 存在output中，开启写事件
    if (!error && n < len) {
        m_outputBuffer.append(static_cast<const char*>(message) + n, len - n);
        if (!m_channel->isWritable()) {
            m_channel->enableWriting();
        }
    }
}
void TcpConnection::shutdown() {
    if (m_state == kConnected) {
        setState(kDisconnecting);
        m_loop->runInLoop(std::bind(&TcpConnection::shutdownInloop, this));
    }
}

void TcpConnection::shutdownInloop() {
    m_loop->assertInThread();
    // 不可写，则关闭
    if (!m_channel->isWritable()) {
        m_connSocket->shutdownWrite();
    }
}

void TcpConnection::setTcpNoDelay(bool on) { m_connSocket->setTcpNoDelay(on); }

void TcpConnection::setKeepAlive(bool on) { m_connSocket->setKeepAlive(on); }

// 把fd的数据读取出来，读到inputBuffer中，并将buffer传给业务逻辑处理函数
void TcpConnection::handleRead(Timestamp receiveTime) {
    m_loop->assertInThread();
    // 从fd将数据读出来
    int saveErrno = 0;
    int n = m_inputBuffer.readFd(m_channel->getFd(), &saveErrno);
    if (n > 0) {
        m_messageCallBack(shared_from_this(), &m_inputBuffer,
                          receiveTime);  // 引用技术+1
    } else if (n == 0) {  // 对端调用shutdown，正常关闭发送FIN， read会返回0
        handleClose();  // 被动关闭连接
    } else {
        errno = saveErrno;
        handleError();
    }
}

void TcpConnection::handleWrite() {
    m_loop->assertInThread();
    if (m_channel->isWritable()) {
        // 把outputBuffer数据写到fd中
        int saveErrno = 0;
        auto n = m_outputBuffer.writeFd(m_channel->getFd(), &saveErrno);
        if (n > 0) {
            // 移动读指针
            m_outputBuffer.moveReadIndex(n);
            if (m_outputBuffer.readableSize() == 0) {  // 一次性写完
                m_channel->disableWriting();
                // 写完是否主动关闭
                shutdown();
            }
        } else {
            // 写出错了？？
        }
    }
}
void TcpConnection::handleError() { m_loop->assertInThread(); }

void TcpConnection::handleClose() {
    m_loop->assertInThread();
    assert(m_state == StateE::kConnected || m_state == StateE::kDisconnecting);
    // we don't close fd, leave it to dtor, so we can find leaks easily.
    setState(kDisconnected);
    m_channel->disableAll();
    m_connectionCallBack(shared_from_this());  // ?
    m_closeCallBack(shared_from_this());
}

}  // namespace mytinywebserver