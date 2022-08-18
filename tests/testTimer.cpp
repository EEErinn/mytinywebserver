#include <sys/eventfd.h>

#include <memory>
#include <string>
#include "../src/log/LogUtils.h"
#include "../src/Timer.h"

using namespace mytinywebserver;

int eventFd() {
    int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (fd < 0) {
    }
    return fd;
}

int main() {
    EventLoop loop;
    std::unique_ptr<TimerManager> timerQueue =
        std::make_unique<TimerManager>(&loop);
    TcpConnectionPtr conn =
        std::make_shared<TcpConnection>(&loop, eventFd(), std::string("OK"));
    LOG_DEBUG << conn << " " << conn.use_count();
    timerQueue->addTimer(
        std::make_shared<TimerNode>(std::make_shared<Entry>(conn), 8));
    
    LOG_DEBUG << "CONN" << conn << " " << conn.use_count();
    loop.loop();
}