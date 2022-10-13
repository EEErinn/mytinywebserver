#pragma once

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "../CurrentThread.h"

namespace mytinywebserver {

//日志级别
class LogLevel {
   public:
    enum Level {
        UNKNOW = 0,
        DEBUG = 1,  //用于开发过程中打印一些运行信息
        INFO = 2,   // 生产环境一些重要信息
        WARN = 3,   // 潜在错误
        ERROR = 4,  // 不影响系统运行错误事件或者异常信息
        FATAL = 5   // 导致程序退出的严重错误
    };

    static const char* toString(LogLevel::Level level);
};

class LogEvent;
class LogFormatter;
class LogAppender;

// 日志器
class Logger : public std::enable_shared_from_this<Logger> {
   public:
    explicit Logger(const std::string& name = "root",
                    std::shared_ptr<LogAppender> appender = nullptr);
    virtual void log(std::shared_ptr<LogEvent> event) = 0;  // 写日志
    virtual ~Logger() {}

    virtual void addAppender(std::shared_ptr<LogAppender> appender);
    void delAppender(std::shared_ptr<LogAppender> appender);

    std::string getName() { return m_name; }
    LogLevel::Level getLevel() const { return m_level; }
    void setLevel(LogLevel::Level val) { m_level = val; }

   protected:
    std::string m_name;                                   // 日志名称
    LogLevel::Level m_level;                              // 日志级别
    std::shared_ptr<LogFormatter> m_defaultFormatter;     // 默认日志格式
    std::list<std::shared_ptr<LogAppender>> m_appenders;  // appender集合
};

class SyncLogger : public Logger {
   public:
    explicit SyncLogger(const std::string& name,
                        std::shared_ptr<LogAppender> appender = nullptr);
    virtual void log(std::shared_ptr<LogEvent> event);  // 直接写入

   private:
    std::shared_ptr<LogAppender> m_defaultAppender;
};

// 类似muduo中的AyncLogging
class AsyncLogger : public Logger {
   public:
    explicit AsyncLogger(const std::string& name,
                         std::shared_ptr<LogAppender> appender = nullptr,
                         int flushInterval = 3);
    void addAppender(std::shared_ptr<LogAppender> appender) override;
    void log(std::shared_ptr<LogEvent> event) override;  // 前端写入缓存
    virtual ~AsyncLogger();

   private:
    void threadFunc();

    int m_flushInterval;          // 日志刷新间隔时间
    std::atomic<bool> m_running;  // 异步线程是否开启
    std::thread m_thread;
    std::mutex m_mutex;
    std::condition_variable condition;
};

}  // namespace mytinywebserver
