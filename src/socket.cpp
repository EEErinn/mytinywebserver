// Copyright©2022 Erin
#include "socket.h"

#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

#include "InetAddress.h"
#include "socketops.h"

namespace mytinywebserver {

Socket::Socket(int fd) : m_sockfd(fd) {}

Socket::~Socket() { socket::closeFd(m_sockfd); }

void Socket::bind(const InetAddress& addr) {
    if (-1 == ::bind(m_sockfd,
                     reinterpret_cast<const sockaddr*>(addr.getSockAddr()),
                     sizeof(sockaddr_in))) {
        // LOG_SYSERR <<
    }
}
void Socket::listen() {
    if (-1 == ::listen(m_sockfd, SOMAXCONN)) {
        // LOG_SYSERR <<
    }
}

int Socket::accept() {
    // LOG address
    sockaddr_in client;
    socklen_t client_addrlen = sizeof(client);
    int fd = ::accept(m_sockfd, reinterpret_cast<sockaddr*>(&client),
                      &client_addrlen);
    if (-1 == fd) {
        // LOG_SYSERR <<
    }
    return fd;
}
void Socket::connect(const InetAddress& addr) {
    if (-1 == ::connect(m_sockfd,
                        reinterpret_cast<const sockaddr*>(addr.getSockAddr()),
                        sizeof(sockaddr_in))) {
        // LOG_SYSERR <<
    }
}

void Socket::shutdownWrite() {
    if (-1 == shutdown(m_sockfd, SHUT_WR)) {
        // LOG_SYSERR <<
    }
}

void Socket::setTcpNoDelay(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(m_sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}

void Socket::setKeepAlive(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(m_sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}

void Socket::setReUse(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

}  // namespace mytinywebserver