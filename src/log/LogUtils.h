#pragma once

#include <memory>
#include <string>

#include "LogAppender.h"
#include "LogEvent.h"
#include "LogFormatter.h"
#include "Logger.h"

extern std::shared_ptr<mytinywebserver::Logger> logger_;

// 使用流式方式将日志级别level写入logger
#define LOG_LEVEL(level)                                                      \
    if (logger_->getLevel() <= level)                                         \
    mytinywebserver::LogEventWrap(                                            \
        logger_, level,                                                       \
        std::shared_ptr<mytinywebserver::LogEvent>(                           \
            new mytinywebserver::LogEvent(                                    \
                __FILE__, __LINE__, 0, mytinywebserver::CurrentThread::tid(), \
                0, time(0), "main")))                                         \
        .getSS()

#define LOG_DEBUG LOG_LEVEL(mytinywebserver::LogLevel::DEBUG)

#define LOG_INFO LOG_LEVEL(mytinywebserver::LogLevel::INFO)

#define LOG_WARN LOG_LEVEL(mytinywebserver::LogLevel::WARN)

#define LOG_ERROR LOG_LEVEL(mytinywebserver::LogLevel::ERROR)

#define LOG_FATAL LOG_LEVEL(mytinywebserver::LogLevel::FATAL)

namespace mytinywebserver {
const char* strerror_tl(int savedErrno);
}