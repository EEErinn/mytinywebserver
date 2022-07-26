// Copyright©2022 Erin
#include "epollpoller.h"

#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "log/LogManager.h"

namespace mytinywebserver {

EpollPoller::EpollPoller()
    : m_epollfd(::epoll_create1(EPOLL_CLOEXEC)),
      m_eventList(m_initEventListSize) {
    if (m_epollfd < 0) {
        LOG_FATAL << "EPollPoller::EPollPoller";
    }
}

EpollPoller::~EpollPoller() { ::close(m_epollfd); }

void EpollPoller::poll(ChannelList* activeChannels) {
    LOG_DEBUG << "fd total count " << m_channelMap.size();
    int ret =
        ::epoll_wait(m_epollfd, &*m_eventList.begin(), m_eventList.size(), -1);
    if (ret < 0) {
        LOG_DEBUG << "nothing happened";
    }
    LOG_DEBUG << ret << " events happened";
    for (int i = 0; i < ret; ++i) {
        Channel* curChannel =
            reinterpret_cast<Channel*>(m_eventList[i].data.ptr);

        int curFd = curChannel->getFd();
        assert(m_channelMap.find(curFd) != m_channelMap.end());
        assert(m_channelMap[curFd] == curChannel);

        curChannel->setRevents(m_eventList[i].events);
        activeChannels->push_back(curChannel);
    }
    if (static_cast<size_t>(ret) == m_eventList.size()) {
        m_eventList.resize(m_eventList.size() * 2);
    }
}

void EpollPoller::update(int op, Channel* channel) {
    epoll_event event;
    ::memset(&event, 0, sizeof(epoll_event));
    event.data.ptr = channel;
    event.events = channel->getEvents();
    LOG_DEBUG << "epoll_ctl op = " << op << " fd = " << channel->getFd()
              << " event = { " << channel->eventsToString() << " }";
    if (::epoll_ctl(m_epollfd, op, channel->getFd(), &event) < 0) {
        LOG_ERROR << "update error";
    }
}

void EpollPoller::updateChannel(Channel* channel) {
    Channel::State index = channel->getIndex();
    int curFd = channel->getFd();
    LOG_DEBUG << "fd = " << channel->getFd()
              << " events = " << channel->getEvents();
    //   << " index = " << index;
    if (index == Channel::State::sNew) {
        assert(m_channelMap.find(curFd) == m_channelMap.end());
        m_channelMap[curFd] = channel;
        channel->setIndex(Channel::State::sAdded);
        update(EPOLL_CTL_ADD, channel);
    } else if (index == Channel::State::sDeled) {
        assert(m_channelMap.find(curFd) != m_channelMap.end());
        assert(m_channelMap[curFd] == channel);
        channel->setIndex(Channel::State::sAdded);
        update(EPOLL_CTL_ADD, channel);
    } else if (index == Channel::State::sAdded) {
        assert(m_channelMap.find(curFd) != m_channelMap.end());
        assert(m_channelMap[curFd] == channel);
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->setIndex(Channel::State::sDeled);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EpollPoller::removeChannel(Channel* channel) {
    int curFd = channel->getFd();
    assert(m_channelMap.find(curFd) != m_channelMap.end());
    assert(m_channelMap[curFd] == channel);
    assert(channel->isNoneEvent());
    int index = channel->getIndex();
    assert(index == Channel::State::sDeled || index == Channel::State::sAdded);

    // 从m_channelMap移除
    int n = m_channelMap.erase(curFd);
    assert(n == 1);

    if (index == Channel::State::sAdded) {
        update(EPOLL_CTL_DEL, channel);
    }

    channel->setIndex(Channel::State::sNew);
}

bool EpollPoller::hasChannel(Channel* channel) {
    auto it = m_channelMap.find(channel->getFd());
    return it != m_channelMap.end() && it->second == channel;
}

}  // namespace mytinywebserver