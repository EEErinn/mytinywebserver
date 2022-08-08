// Copyright©2022 Erin
#include "socketops.h"

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

namespace mytinywebserver {
namespace socket {

int setNonBlocking(int fd) {
    int old_option = fcntl(fd, F_GETFD);
    int new_option = old_option | O_NONBLOCK | O_CLOEXEC;
    fcntl(fd, F_SETFD, new_option);
    return old_option;
}

int createSocketFd() {
    int fd = ::socket(PF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        // LOG_
    }
    setNonBlocking(fd);
    return fd;
}

void closeFd(int fd){
    if (::close(fd) < 0) {
        // LOG_SYSERR << "sockets::close";
    }
}

}  // namespace socket
}  // namespace mytinywebserver