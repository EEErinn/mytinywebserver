// Copyright©2022 Erin
#include <string.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include "../src/Timestamp.h"
#include "../src/channel.h"
#include "../src/eventloop.h"

// 测试eventloop、channel、poller的实现逻辑是否正确
using namespace mytinywebserver;

EventLoop* g_loop;

void timeout(Timestamp receiveTime) {
    printf("Timeout");
    g_loop->quit();
}

int main() {
    EventLoop loop;
    g_loop = &loop;

    int timefd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    Channel* channel = new Channel(&loop, timefd);
    channel->setReadCallBack_(std::bind(timeout, Timestamp::now()));
    channel->enableReading();

    struct itimerspec howlong;
    ::memset(&howlong, 0, sizeof(howlong));
    howlong.it_value.tv_sec = 5;
    ::timerfd_settime(timefd, 0, &howlong, NULL);

    loop.loop();
    ::close(timefd);
}