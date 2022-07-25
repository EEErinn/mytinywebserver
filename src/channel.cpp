// Copyright©2022 Erin
#include "channel.h"

#include <sys/epoll.h>

#include "eventloop.h"

namespace mytinywebserver {

const int Channel::m_NoneEvent = 0;
const int Channel::m_ReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::m_WriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* loop, int fd)
    : m_loop(loop),
      m_fd(fd),
      m_events(0),
      m_revents(0),
      m_index(Channel::State::sNew),
      m_tied(false) {}

Channel::~Channel() {
    // if (m_loop->hasChannel(this)) removeSelf();
    // assert(!m_loop->hasChannel(this));
}

// channel update remove -> eventloop update remove -> poller update remove
void Channel::updateSelf() { m_loop->updateChannel(this); }
void Channel::removeSelf() { m_loop->removeChannel(this); }

void Channel::enableReading() {
    m_events |= m_ReadEvent;
    updateSelf();
}
void Channel::enableWriting() {
    m_events |= m_WriteEvent;
    updateSelf();
}
void Channel::disableReading() {
    m_events &= ~m_ReadEvent;
    updateSelf();
}
void Channel::disableWriting() {
    m_events &= ~m_WriteEvent;
    updateSelf();
}
void Channel::disableAll() {
    m_events = m_NoneEvent;
    updateSelf();
}

/**
 * @brief 拥有channel的对象若不能保证自身生命周期比channel长，则调用tie
 * eg：TcpConnection中注册了Chnanel对应的回调函数，传入的回调函数均为TcpConnection对象的成员方法，
 * 因此：Channel的结束一定早于TcpConnection对象！
 * 此处用tie去解决TcoConnection和Channel的生命周期时长问题，从而保证了Channel对象能够在TcpConnection销毁前销毁。
 */
void Channel::tie(const std::shared_ptr<void>& v) {
    m_tie = v;
    m_tied = true;
}

// EPOLLPRI: 高优先级数据可读，如tcp带外数据
// EPOLLHUP：挂起。管道的写端被关闭后，读端描述符上收到EPOLLHUP信号
// EPOLLRDHUP：tcp连接被对方关闭，或者对方关闭了写操作
void Channel::handleEventWithGuard(Timestamp receiveTime) {
    if ((m_revents & EPOLLHUP) && !(m_revents & EPOLLHUP)) {
        if (m_closeCallBack_) m_closeCallBack_();
    } else if (m_revents & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (m_readCallBack_) m_readCallBack_();
    } else if (m_revents & EPOLLOUT) {
        if (m_writeCallBack_) m_writeCallBack_();
    } else if (m_revents & EPOLLERR) {
        if (m_errorCallBack_) m_errorCallBack_();
    }
}

void Channel::handleEvent(Timestamp receiveTime) {
    if (m_tied) {
        std::shared_ptr<void> guard = m_tie.lock();
        if (guard) {
            handleEventWithGuard(receiveTime);
        }
        // 如果提升失败了 就不做任何处理
        // 说明Channel的所有者对象已经不存在了
    } else {
        handleEventWithGuard(receiveTime);
    }
}

}  // namespace mytinywebserver
