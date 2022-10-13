#include "TcpConnection.h"

#include <unistd.h>

#include "Timer.h"
#include "log/LogManager.h"
#include "socketops.h"

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
    LOG_DEBUG << "TcpConnection::ctor[" << m_name << "] at " << this
              << " fd=" << fd;
    m_connSocket->setKeepAlive(true);
    m_connSocket->setTcpNoDelay(true);
}

TcpConnection::~TcpConnection() {
    LOG_DEBUG << "TcpConnection::dtor[" << m_name << "] at " << this
              << " fd=" << m_channel->getFd();
    assert(m_state == kDisconnected);
}

void TcpConnection::connectEstablished() {
    LOG_DEBUG << "TcpConnection::connectEstablished [" << m_name << "] at "
              << this << " fd=" << m_channel->getFd();
    assert(m_state == StateE::kconnecting);
    setState(StateE::kConnected);
    m_channel->tie(shared_from_this());
    m_channel->enableReading();
    m_connectionCallBack(shared_from_this());
}

void TcpConnection::newConnTimer(TimerManager* timerQueue, int timeout) {
    std::shared_ptr<Entry> entry(new Entry(shared_from_this()));
    std::shared_ptr<TimerNode> timer(new TimerNode(entry, timeout));
    timerQueue->addTimer(timer);
    std::weak_ptr<Entry> weakEntry(entry);
    setWeakPtr(weakEntry);
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
    if (m_state == kConnected) {
        if (m_loop->isInloopThread()) {
            sendInloop(message, len);
        } else {
            void (TcpConnection::*fp)(const void* message, size_t len) =
                &TcpConnection::sendInloop;
            m_loop->runInLoop(std::bind(fp, this, message, len));
        }
    }
}
void TcpConnection::send(const std::string& message) {
    send(message.data(), message.size());
}

void TcpConnection::send(Buffer* buf) {
    if (m_state == kConnected) {
        if (m_loop->isInloopThread()) {
            sendInloop(buf->beginRead(), buf->readableSize());
            buf->moveToFront();
        } else {
            void (TcpConnection::*fp)(const std::string& message) =
                &TcpConnection::sendInloop;
            m_loop->runInLoop(std::bind(fp, this, buf->retrieveAllAsString()));
        }
    }
}

void TcpConnection::sendInloop(const std::string& message) {
    sendInloop(message.data(), message.size());
}

void TcpConnection::sendInloop(const void* message, size_t len) {
    m_loop->assertInThread();
    // 先直接发送，若没有发送完，则存在output中，开启写事件
    size_t n = 0;
    bool error = false;
    if (m_state == kDisconnected) {
        LOG_WARN << "disconnected, give up writing";
        return;
    }
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
                    LOG_ERROR << "TcpConnection::sendInLoop";
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
void TcpConnection::forceClose() {
    // FIXME: use compare and swap
    if (m_state == kConnected || m_state == kDisconnecting) {
        setState(kDisconnecting);
        m_loop->queueInLoop(
            std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
    }
}

void TcpConnection::forceCloseInLoop() {
    m_loop->assertInThread();
    if (m_state == kConnected || m_state == kDisconnecting) {
        // as if we received 0 byte in handleRead();
        handleClose();
    }
}

void TcpConnection::shutdown() {
    LOG_DEBUG << "TcpConnection::shutdown";
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
    LOG_DEBUG << "TcpConnection::handleRead";
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
            LOG_ERROR << "TcpConnection::handleWrite";
        }
    } else {
        LOG_DEBUG << "Connection fd = " << m_channel->getFd()
                  << " is down, no more writing";
    }
}

void TcpConnection::handleError() {
    m_loop->assertInThread();
    int err = socket::getSocketError(m_channel->getFd());
    LOG_ERROR << "TcpConnection::handleError [" << m_name
              << "] - SO_ERROR = " << err;
}

void TcpConnection::handleClose() {
    m_loop->assertInThread();
    LOG_DEBUG << "fd = " << m_channel->getFd() << " is closing";
    assert(m_state == StateE::kConnected || m_state == StateE::kDisconnecting);
    // we don't close fd, leave it to dtor, so we can find leaks easily.
    setState(kDisconnected);
    m_channel->disableAll();
    m_connectionCallBack(shared_from_this());  // ?
    m_closeCallBack(shared_from_this());
}

}  // namespace mytinywebserver
