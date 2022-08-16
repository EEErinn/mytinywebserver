#pragma once
#include <fstream>
#include <memory>

#include "LogEvent.h"
#include "LogFormatter.h"
#include "Logger.h"
namespace mytinywebserver {

//日志输出地
class LogAppender {
   public:
    using ptr = std::shared_ptr<LogAppender>;
    LogAppender() {}
    virtual ~LogAppender() {}

    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level,
                     std::shared_ptr<LogEvent> event) = 0;
    virtual void flush() {}

    LogFormatter::ptr getFormatter() { return m_formatter; }
    LogLevel::Level getLevel() const { return m_level; }

    void setFormatter(LogFormatter::ptr val) { m_formatter = val; }
    void setLevel(LogLevel::Level val) { m_level = val; }

   protected:
    LogLevel::Level m_level = LogLevel::Level::DEBUG;
    // 日志格式器
    LogFormatter::ptr m_formatter;
};

// 输出到控制台
class StdoutLogAppender : public LogAppender {
   public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level,
                     std::shared_ptr<LogEvent> event) override {
        if (level >= m_level) {
            std::cout << m_formatter->format(logger, level, event);
        }
    }
};

// 输出到文件
class FileLogAppender : public LogAppender {
   public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    FileLogAppender(const std::string& filename) : m_filename(filename) {
        if (!reopen()) throw std::exception();
    }
    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level,
                     std::shared_ptr<LogEvent> event) override {
        if (level >= m_level) {
            m_filestream << m_formatter->format(logger, level, event);
        }
    }
    bool reopen();
    ~FileLogAppender() {
        if (m_filestream) m_filestream.close();
    }

   private:
    std::string m_filename;
    std::ofstream m_filestream;
};

}  // namespace mytinywebserver
