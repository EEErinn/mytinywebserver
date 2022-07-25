// Copyright©2022 Erin
#include "epollpoller.h"
#include "poller.h"

namespace mytinywebserver {

Poller* Poller::newDefaultPoller() {
    if (::getenv("WEBSERVER_USE_POLL")) {
        return nullptr;
    }
    return new EpollPoller();
}

}  // namespace mytinywebserver
