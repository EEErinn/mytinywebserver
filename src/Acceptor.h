// Copyright©2022 Erin
#pragma once

#include <functional>
#include <memory>

#include "InetAddress.h"
#include "socket.h"
#include "Timestamp.h"

namespace mytinywebserver {

class Channel;
class EventLoop;

// TCP server拥有该对象，在主线程bind、listen
// 封装listen sock。
// channel read事件 handleRead - accept
class Acceptor {
   public:
    using NewConnectionCallBack_ = std::function<void(int)>;

    Acceptor(EventLoop* loop, const InetAddress& addr);
    ~Acceptor();

    void setNewconnectionCallBack(NewConnectionCallBack_ v) {
        m_newConnectionCallBack = v;
    }

    void listen();
    bool isListenning() { return m_listening; }

   private:
    void handleRead(Timestamp receiveTime);

    EventLoop* m_loop;  // mainloop 主线程
    Socket m_listenSock;
    bool m_listening;                    // 是否在监听
    std::unique_ptr<Channel> m_channel;  // listen socket对应的channel
    NewConnectionCallBack_ m_newConnectionCallBack;  // 连接后执行的回调函数
};

}  // namespace mytinywebserver
