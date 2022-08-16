#include "Logger.h"

#include <map>
#include <tuple>

#include "LogAppender.h"
#include "LogEvent.h"
#include "LogFormatter.h"

namespace mytinywebserver {

Logger::Logger(const std::string& filePath, const std::string& name)
    : m_name(name), m_level(LogLevel::Level::DEBUG) {
    m_formatter.reset(new LogFormatter(
        "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l:%T%m%n"));
    addAppender(LogAppender::ptr(new StdoutLogAppender()));
    addAppender(LogAppender::ptr(new FileLogAppender(filePath)));
}

void Logger::log(LogLevel::Level level,
                 std::shared_ptr<LogEvent> event) {  // note
    if (level >= m_level) {
        auto self = shared_from_this();
        for (auto& i : m_appenders) {
            i->log(self, level, event);
        }
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

void Logger::addAppender(LogAppender::ptr appender) {
    if (!appender->getFormatter()) {
        appender->setFormatter(m_formatter);
    }
    m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender) {
    for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it) {
        if (*it == appender) {
            m_appenders.erase(it);
            break;
        }
    }
}

}  // namespace mytinywebserver
