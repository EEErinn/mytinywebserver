// Copyright©2022 Erin
#pragma once
#include <sys/epoll.h>

#include "channel.h"
#include "poller.h"

namespace mytinywebserver {

class EpollPoller : public Poller {
   public:
    EpollPoller();
    ~EpollPoller() override;

    void poll(int fd, ChannelList* activeChannels);

    void updateChannel(Channel* channel) override;
    // 从poll监测表移除, 从m_channelMap中移除，状态置为sNew
    void removeChannel(Channel* channel) override;

   private:
    void update(int op, Channel* channel);

    int m_epollfd;  // epoll_create创建返回的fd保存在epollfd_中
    static const int m_initEventListSize = 16;

    using EventList = std::vector<epoll_event>;
    // 每次调用epoll都需要一个文件描述符事件集作为参数，接受epoll_wait的返回。因此缓存起来
    EventList m_eventList;
};

}  // namespace mytinywebserver