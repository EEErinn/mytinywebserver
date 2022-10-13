#include "Timer.h"

#include <sys/timerfd.h>

#include "log/LogManager.h"
namespace mytinywebserver {
int createTimerfd() {
    int timeFd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timeFd < 0) {
        LOG_FATAL << "Failed in timerfd_create";
    }
    return timeFd;
}

Entry::Entry(const WeakTcpConnectionPtr& weakConn) : m_weakConn(weakConn) {
    LOG_DEBUG << "Entry ctor" << weakConn.expired();
}

Entry::~Entry() {
    LOG_DEBUG << "Entry::~Entry dtor";
    auto conn = m_weakConn.lock();
    if (conn) conn->forceClose();
}

TimerNode::TimerNode(std::shared_ptr<Entry> conn, int timeout)
    : m_expire(addTime(Timestamp::now(), timeout)), m_conn(conn) {}

TimerManager::TimerManager(EventLoop* loop)
    : m_loop(loop),
      m_timerFd(createTimerfd()),
      m_timerChannel(loop, m_timerFd) {
    LOG_DEBUG << "TimerManager::ctor";
    m_timerChannel.setReadCallBack_(std::bind(&TimerManager::handleRead, this));
    m_timerChannel.enableReading();

    LOG_DEBUG << "set timer";
    setTimePer(m_timerFd);
}

TimerManager::~TimerManager() {}

void TimerManager::setTimePer(int fd) {
    struct itimerspec newValue;
    memset(&newValue, 0, sizeof(newValue));
    newValue.it_interval = {5, 0};
    newValue.it_value = {5, 100};  //到期时间距离当前时间的间隔
    int ret = ::timerfd_settime(fd, 0, &newValue, 0);
    if (ret == -1) {
        LOG_FATAL << "timerfd_settime failed";
    }
}

void TimerManager::addTimer(TimerNode::TimerPtr v) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_queue.push(v); 
}

void TimerManager::handleRead() {
    LOG_DEBUG << "Timer::handleRead";
    uint64_t val;
    int ret = read(m_timerFd, &val, sizeof(val));
    if (ret != sizeof(val)) {
        LOG_ERROR << "read " << ret << "bytes instead of 8 frome timerfd";
    }
    Timestamp curTime = Timestamp::now();
    std::unique_lock<std::mutex> lock(m_mutex);
    while (!m_queue.empty()) {
        if (m_queue.top()->getExpTime() < curTime) {
            LOG_DEBUG << "TCPconnection timeout";
            m_queue.pop();
        } else {
            return;
        }
    }
}
}  // namespace mytinywebserver
