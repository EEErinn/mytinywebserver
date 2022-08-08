#pragma once

#include <memory>
#include <string>

#include "Buffer.h"
#include "CallBacks.h"
#include "InetAddress.h"
#include "channel.h"
#include "eventloop.h"
#include "socket.h"

namespace mytinywebserver {

/**
 * @brief TcpConnection生命周期不稳定
 * 客户端断开某个tcpsocket，对应的服务端进程中的Tcpconnection对象也即将销毁，
 * 但不能立马delete，因为其他地方可能持有它的引用，因此需要引用计数。
 *
 * 想用shared_from_this，那么
 * tcpserver调用newConnection时需要使用shared_ptr创建TcpConnection 和线程安全
 */
class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
   public:
    TcpConnection(EventLoop* loop, int fd, const std::string& name);
    ~TcpConnection();

    // 用于tcpserver接受一个新的连接后
    void connectEstablished();
    // 用于tcpserver将其从map中移除后，通知用户连接已断开，是析构前的最后一个成员函数。
    void connectDestoryed();

    std::string getName() const { return m_name; }
    EventLoop* getLoop() const { return m_loop; }

    void setCloseCallBack(CloseCallBack_ v) { m_closeCallBack = v; }
    void setConnectionCallBack(ConnectionCallBack_ v) {
        m_connectionCallBack = v;
    }
    void setMessageCallBack(MessageCallBack_ v) { m_messageCallBack = v; }

    // 发送数据, 可跨线程
    void send(const void* message, size_t len);
    void send(const std::string& message);
    void sendInloop(const void* message, size_t len);

    // 主动关闭连接， 可跨线程
    void shutdown();
    void shutdownInloop();

    // 設置socket option
    void setTcpNoDelay(bool on);
    void setKeepAlive(bool on);

   private:
    enum StateE { kconnecting, kConnected, kDisconnected, kDisconnecting };

    void setState(StateE v) { m_state = v; }

    // 事件处理函数
    void handleRead(Timestamp);
    void handleWrite();
    void handleError();
    void handleClose();

    EventLoop* m_loop;         // 连接所在loop
    const std::string m_name;  // 连接名称
    StateE m_state;            // 连接状态
    std::unique_ptr<Socket> m_connSocket;
    std::unique_ptr<Channel> m_channel;
    // 缓冲区
    Buffer m_inputBuffer;  // 客户端发送数据，服务端读入数据到m_inputBuffer
    Buffer m_outputBuffer;  // 服务端写数据到m_outputBuffer，输出数据
    // 业务逻辑函数
    ConnectionCallBack_ m_connectionCallBack;  // 连接建立后执行
    MessageCallBack_ m_messageCallBack;  // 读事件后执行, 解析读到的inputBuffer
    // 由tcpserver设置
    CloseCallBack_ m_closeCallBack;
    // 本端和对端的地址
    // const InetAddress m_localAddr;
    // const InetAddress m_peerAddr;
};

}  // namespace mytinywebserver