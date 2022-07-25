// Copyright©2022 Erin
#pragma once

#include <unordered_map>
#include <vector>

#include "channel.h"

namespace mytinywebserver {

// 只有eventloop拥有poller对象，保证poller所属的eventloop是线程安全的即可，因此poller无需加锁。其生命周期同eventloop
// Poller不拥有channel，因此channel在析构函数之前需要调用removeself，避免poller中存在channel的空悬指针
class Poller {
   public:
    using ChannelList = std::vector<Channel*>;

    Poller() = default;
    virtual ~Poller() {}

    virtual void poll(int fd, ChannelList* activeChannels) = 0;
    // 更新或修改channel信息
    virtual void updateChannel(Channel* channel) = 0;
    // 删除channel中fd上注册的事件，并从m_channelMap中移除
    virtual void removeChannel(Channel* channel) = 0;

    virtual bool hasChannel(Channel* channel);

    static Poller* newDefaultPoller();

   protected:
    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap m_channelMap;
};

}  // namespace mytinywebserver