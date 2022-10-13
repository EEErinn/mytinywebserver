#include "Logger.h"

#include <map>
#include <tuple>

#include "LogAppender.h"
#include "LogEvent.h"
#include "LogFormatter.h"

namespace mytinywebserver {
// "%d{%Y-%m-%d %H:%M:%S}%T%t%T[%p]%T[%c]%T%f:%l%T%m%n"
Logger::Logger(const std::string& name, std::shared_ptr<LogAppender> appender)
    : m_name(name), m_level(LogLevel::Level::UNKNOW) {
    m_defaultFormatter.reset(
        new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T[%p]%T[%c]%T%f:%l%T%m%n"));
    if (appender) {
        if (!appender->getFormatter()) {
            appender->setFormatter(m_defaultFormatter);
        }
        m_appenders.push_back(appender);
    }
}

const char* LogLevel::toString(LogLevel::Level level) {
    switch (level) {
#define XX(name)                \
    case LogLevel::Level::name: \
        return #name;
        break;
        XX(DEBUG);
        XX(INFO);
        XX(WARN);
        XX(ERROR);
        XX(FATAL);
#undef XX
        default:
            return "UNKNOW";
    }
}

void Logger::addAppender(std::shared_ptr<LogAppender> appender) {
    if (!appender->getFormatter()) {
        appender->setFormatter(m_defaultFormatter);
    }
    m_appenders.push_back(appender);
}

void Logger::delAppender(std::shared_ptr<LogAppender> appender) {
    for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it) {
        if (*it == appender) {
            m_appenders.erase(it);
            break;
        }
    }
}

SyncLogger::SyncLogger(const std::string& name,
                       std::shared_ptr<LogAppender> appender)
    : Logger(name, appender) {
    m_defaultAppender = std::make_shared<StdoutLogAppender>();
    m_defaultAppender->setFormatter(m_defaultFormatter);
}

void SyncLogger::log(std::shared_ptr<LogEvent> event) {
    auto level = event->getLevel();
    if (level >= m_level) {
        auto self = shared_from_this();
        if (!m_appenders.empty()) {
            for (auto& i : m_appenders) {
                i->log(self, event);
            }
        } else {
            m_defaultAppender->log(self, event);
        }
    }
}

AsyncLogger::AsyncLogger(const std::string& name,
                         std::shared_ptr<LogAppender> appender,
                         int flushInterval)
    : Logger(name, appender),
      m_flushInterval(flushInterval),
      m_running(true),
      m_thread(std::thread(std::bind(&AsyncLogger::threadFunc, this))) {}

AsyncLogger::~AsyncLogger() {
    m_running = false;
    condition.notify_all();
    m_thread.join();
}

void AsyncLogger::addAppender(std::shared_ptr<LogAppender> appender) {
    if (!appender->getFormatter()) {
        appender->setFormatter(m_defaultFormatter);
    }
    appender->setNotifyFunc([this] { condition.notify_one(); });
    m_appenders.push_back(appender);
}

// 其他线程会调用，加锁保证线程安全
void AsyncLogger::log(std::shared_ptr<LogEvent> event) {
    auto level = event->getLevel();
    if (level >= m_level) {
        auto self = shared_from_this();
        if (!m_appenders.empty()) {
            for (auto& i : m_appenders) {
                i->log(self, event);  // 写入缓存
            }
        }
    }
}

void AsyncLogger::threadFunc() {
    while (m_running) {
        std::unique_lock<std::mutex> lock(m_mutex);
        // 隔flushInterval刷新一下
        condition.wait_for(lock, std::chrono::seconds(m_flushInterval));
        for (auto& i : m_appenders) {
            i->flush();  // 缓存写到对应的目的地
        }
    }
}

}  // namespace mytinywebserver
