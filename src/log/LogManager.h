#pragma once

#include <memory>
#include <string>

#include "LogAppender.h"
#include "LogEvent.h"
#include "LogFormatter.h"
#include "Logger.h"

extern bool log_open;
extern bool isAsync;
extern std::shared_ptr<mytinywebserver::Logger> logger_;

// 使用流式方式将日志级别level写入logger
#define LOG_LEVEL(level)                                               \
    if (log_open)                                                      \
    mytinywebserver::LogEventWrap(                                     \
        logger_,                                                       \
        std::shared_ptr<mytinywebserver::LogEvent>(                    \
            new mytinywebserver::LogEvent(                             \
                level, logger_->getName().c_str(), __FILE__, __LINE__, \
                mytinywebserver::CurrentThread::tid(), time(0))))      \
        .getSS()

#define LOG_DEBUG LOG_LEVEL(mytinywebserver::LogLevel::DEBUG)

#define LOG_INFO LOG_LEVEL(mytinywebserver::LogLevel::INFO)

#define LOG_WARN LOG_LEVEL(mytinywebserver::LogLevel::WARN)

#define LOG_ERROR LOG_LEVEL(mytinywebserver::LogLevel::ERROR)

#define LOG_FATAL LOG_LEVEL(mytinywebserver::LogLevel::FATAL)

namespace mytinywebserver {

namespace detail {
static const std::string default_sync_log_file = "app_sync_log.txt";
static const std::string default_async_log_file = "app_async";
static const int default_roll_size = 50 * 1000 * 1000;
static const int default_flush_interval = 3;
}  // namespace detail

class LoggerManager : public noncopyable {
   public:
    static LoggerManager& getInstance() {
        static LoggerManager manager;
        return manager;
    }

    // 默认同步日志器 写入到文件
    static std::shared_ptr<Logger> getSyncFileRoot() {
        if (log_open == false) {
            return nullptr;
        }

        static std::shared_ptr<Logger> sync_root = std::make_shared<SyncLogger>(
            "sync_file_root", std::make_shared<SyncFileLogAppender>(
                                  std::string(detail::default_sync_log_file)));
        return sync_root;
    }

    // 默认异步日志器 写入到文件
    static std::shared_ptr<Logger> getAsyncFileRoot() {
        if (log_open == false) {
            return nullptr;
        }

        static std::shared_ptr<Logger> async_root =
            std::make_shared<AsyncLogger>(
                "async_file_root",
                std::make_shared<AyncFileLogAppender>(
                    detail::default_async_log_file, detail::default_roll_size),
                detail::default_flush_interval);

        return async_root;
    }

    // 默认同步日志器 写入到标准输出
    static std::shared_ptr<Logger> getSyncStdoutRoot() {
        if (log_open == false) {
            return nullptr;
        }

        static std::shared_ptr<Logger> sync_root =
            std::make_shared<SyncLogger>("sync_stdout_root");
        return sync_root;
    }

   private:
    std::mutex mutex_;
    std::map<std::string, std::shared_ptr<Logger>> loggers_;
};
}  // namespace mytinywebserver