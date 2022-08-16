#pragma once

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "../CurrentThread.h"
#include "singleton.h"

namespace mytinywebserver {

class Logger;

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
class Logger : public Singleton<Logger>,
               public std::enable_shared_from_this<Logger> {
   public:
    void log(LogLevel::Level level, std::shared_ptr<LogEvent> event);

    void addAppender(std::shared_ptr<LogAppender> appender);
    void delAppender(std::shared_ptr<LogAppender> appender);

    std::string getName() { return m_name; }
    LogLevel::Level getLevel() const { return m_level; }
    void setLevel(LogLevel::Level val) { m_level = val; }
    void setFilePath(const std::string& path) { m_filePath = path;}

   private:
    explicit Logger(const std::string& filePath = "./log.txt",
                    const std::string& name = "root");

    std::string m_name;
    std::string m_filePath;
    LogLevel::Level m_level;
    std::list<std::shared_ptr<LogAppender>> m_appenders;
    std::shared_ptr<LogFormatter> m_formatter;
    friend Singleton<Logger>;
};

}  // namespace mytinywebserver
