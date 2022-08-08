// Copyright©2022 Erin
#pragma once

#include <optional>
// 封装socket fd的生命周期。
// bind、listen、accept、close
// 设置sock选项
namespace mytinywebserver {

class InetAddress;

class Socket {
   public:
    Socket(int fd);
    ~Socket();

    void bind(const InetAddress& addr);
    void listen();
    int accept();
    void connect(const InetAddress& addr);
    void shutdownWrite();

    int getSockFd() { return m_sockfd; }

    // set socket option info
    void setTcpNoDelay(bool on);
    void setKeepAlive(bool on);

   private:
    const int m_sockfd;
};

}  // namespace mytinywebserver