#pragma once

#include <netinet/in.h>
#include <sys/un.h>

#include <string>

namespace mytinywebserver {

// 封装socket地址，目前暂只支持ipv4。完成主机序和网络序的转换
class InetAddress {
   public:
    InetAddress(const char* ip, int port);
    explicit InetAddress(const struct sockaddr_in& addr) : m_addr(addr) {}

    std::string toIp() const;
    uint16_t toPort() const;
    std::string toString() const;

    const sockaddr_in* getSockAddr() const { return &m_addr; }
    void setSockAddr(const sockaddr_in& v) { m_addr = v; }

   private:
    sockaddr_in m_addr;
};

}  // namespace mytinywebserver
