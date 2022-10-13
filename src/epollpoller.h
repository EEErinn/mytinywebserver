// Copyright©2022 Erin
#pragma once
#include <sys/epoll.h>

#include <unordered_map>
#include <vector>

#include "channel.h"

namespace mytinywebserver {
// 只有eventloop拥有poller对象，保证poller所属的eventloop是线程安全的即可，因此poller无需加锁。其生命周期同eventloop
// Poller不拥有channel，因此channel在析构函数之前需要调用removeself，避免poller中存在channel的空悬指针
class EpollPoller {
   public:
    using ChannelList = std::vector<Channel*>;
    EpollPoller();
    ~EpollPoller();

    void poll(ChannelList* activeChannels);
    // 更新或修改channel信息
    void updateChannel(Channel* channel);
    // 从poll监测表移除, 从m_channelMap中移除，状态置为sNew
    void removeChannel(Channel* channel);

    bool hasChannel(Channel* channel);

   private:
    void update(int op, Channel* channel);

    int m_epollfd;  // epoll_create创建返回的fd保存在epollfd_中
    static const int m_initEventListSize = 16;

    using EventList = std::vector<epoll_event>;
    // 每次调用epoll都需要一个文件描述符事件集作为参数，接受epoll_wait的返回。因此缓存起来
    EventList m_eventList;

    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap m_channelMap;
};

}  // namespace mytinywebserver