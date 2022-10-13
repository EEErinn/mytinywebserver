#include "InetAddress.h"

#include <arpa/inet.h>
// #include <sstream>

namespace mytinywebserver {
InetAddress::InetAddress(const char* ip, int port) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    inet_pton(m_addr.sin_family, ip, &m_addr.sin_addr);
    m_addr.sin_port = htons(port);
}

std::string InetAddress::toIp() const {
    char buf[60] = {'\0'};
    ::inet_ntop(AF_INET, &m_addr.sin_addr, buf, sizeof(buf));
    return buf;
}
uint16_t InetAddress::toPort() const { return ntohs(m_addr.sin_port); }

std::string InetAddress::toString() const {
    // std::string ip = toIp();
    // std::ostringstream os;
    // os << toPort();
    // return ip + os.str();
    char buf[60] = {'\0'};
    ::inet_ntop(AF_INET, &m_addr.sin_addr, buf, sizeof(buf));
    size_t end = ::strlen(buf);
    uint16_t port = ntohs(m_addr.sin_port);
    sprintf(buf + end, "%u", port);
    return buf;
}
}  // namespace mytinywebserver