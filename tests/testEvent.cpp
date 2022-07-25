// Copyright©2022 Erin
#include <pthread.h>

#include <iostream>
#include <mutex>
#include <thread>

#include "../src/CurrentThread.h"
#include "../src/eventloop.h"

// 验证eventloop是否线程安全
using namespace mytinywebserver;

EventLoop* g_loop = nullptr;
std::mutex g_mutex;

void print(EventLoop* loop) {
    std::unique_lock<std::mutex> lock(g_mutex);
    std::cout << loop << " was created in threadId_ = " << loop->getThreadId()
              << ", current thread id = " << CurrentThread::tid() << std::endl;
    std::cout.flush();
}

void threadFunc() {
    print(g_loop);
    std::unique_lock<std::mutex> lock(g_mutex);
    g_loop->loop();
    // sleep(60000);
}

int main() {
    // 主线程创建的eventloop
    EventLoop eventloop;
    {
        std::unique_lock<std::mutex> lock(g_mutex);
        g_loop = &eventloop;
    }
    print(&eventloop);
    std::thread t1(threadFunc);
    t1.join();
    // eventloop.loop();
}