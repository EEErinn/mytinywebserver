// Copyright©2022 Erin
#include "poller.h"

namespace mytinywebserver {

bool Poller::hasChannel(Channel* channel) {
    auto it = m_channelMap.find(channel->getFd());
    return it != m_channelMap.end() && it->second == channel;
}

}  // namespace mytinywebserver