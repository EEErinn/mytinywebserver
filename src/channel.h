// Copyright©2022 Erin
#pragma once
#include <assert.h>

#include <functional>
#include <memory>
#include <utility>

#include "Timestamp.h"
#include "noncopyable.h"

namespace mytinywebserver {

class EventLoop;
// channel不包含poller，所以通过eventloop通讯
// channel的成员函数都只能在I/O线程调用，因此更新数据成员不需要加锁
class Channel : noncopyable {
   public:
    Channel(EventLoop* loop, int fd);
    ~Channel();  // 构造函数之前，必须调用removeself，将自身从epoll的m_channelMap中移除,
                 // 避免产生空悬指针

    void removeSelf();  // 从epollfd的监听表移除，但仍然在epoller的channels_里

    // 设置fd上的事件
    void enableReading();   // 向poller注册读事件
    void enableWriting();   // 向poller注册写事件
    void disableReading();  // 从poller移除读事件
    void disableWriting();  // 从poller移除写事件
    void disableAll();      // 从poller移除所有事件

    bool isWritable() const { return m_events & m_WriteEvent; }
    bool isReadable() const { return m_events & m_ReadEvent; }

    using ReadCallBack_ = std::function<void(Timestamp)>;
    using EventCallBack_ = std::function<void()>;

    // 事件处理函数:核心函数，根据m_revents的值分别调用不同用户回调。
    void handleEvent(Timestamp receiveTime);
    // 保存拥有channel的对象指针，防止在事件处理过程中对象销毁，而eventloop还为其在事件处理
    void tie(const std::shared_ptr<void>&);

    // 设置读信号的回调
    void setReadCallBack_(ReadCallBack_ v) { m_readCallBack_ = std::move(v); }
    // 设置写信号的回调
    void setWriteCallBack_(EventCallBack_ v) {
        m_writeCallBack_ = std::move(v);
    }
    // 设置关闭信号的回调
    void setCloseCallBack_(EventCallBack_ v) {
        m_closeCallBack_ = std::move(v);
    }
    // 设置错误信号的回调
    void setErrorCallBack_(EventCallBack_ v) {
        m_errorCallBack_ = std::move(v);
    }

    enum State {
        sNew,    // 新建
        sAdded,  // 已添加
        sDeled,  // 已移除:
                 // 已从poll监测的事件集中移除，但poller的m_channelMap中还保留channel对象
    };

    // getter
    EventLoop* getLoop() const { return m_loop; }
    int getFd() const { return m_fd; }
    int getEvents() const { return m_events; }
    int getREvents() const { return m_revents; }
    State getIndex() const { return m_index; }
    bool isNoneEvent() { return m_events == m_NoneEvent; }

    // setter
    void setLoop(EventLoop* val) { m_loop = val; }
    void setEvents(int val) { m_events = val; }
    void setRevents(int val) { m_revents = val; }
    void setIndex(State val) { m_index = val; }

   private:
    void updateSelf();
    void handleEventWithGuard(Timestamp receiveTime);

    EventLoop* m_loop;  // 拥有channel的loop对象
    const int m_fd;     // 封装fd
    int m_events;       // 关心的I/O事件, 由用户设置
    int m_revents;      // 实际发生的事件，由poller设置
    State m_index;      // 在poller中的状态

    static const int m_NoneEvent;
    static const int m_ReadEvent;
    static const int m_WriteEvent;

    // 解决拥有channel的对象和channel的生命周期时长问题
    std::weak_ptr<void> m_tie;
    bool m_tied;

    ReadCallBack_ m_readCallBack_;
    EventCallBack_ m_writeCallBack_;
    EventCallBack_ m_closeCallBack_;
    EventCallBack_ m_errorCallBack_;
};

}  // namespace mytinywebserver
