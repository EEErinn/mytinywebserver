#pragma once
#include <memory>
#include <sstream>
#include <string>

#include "Logger.h"

namespace mytinywebserver {

// class Logger;
//日志事件
class LogEvent {
   public:
    LogEvent(LogLevel::Level level, const char* loggername, const char* file,
             int32_t line, uint32_t thread_id, uint64_t time);

    // getter
    const char* getLoggername() const { return m_loggername; }
    const char* getFile() const { return m_file; }
    int32_t getLine() const { return m_line; }
    uint32_t getThreadId() const { return m_threadId; }
    uint64_t getTime() const { return m_time; }
    std::string getContent() const { return m_ss.str(); }
    std::stringstream& getSS() { return m_ss; }
    LogLevel::Level getLevel() { return m_level; }

    // 格式化写入日志内容
    void format(std::string fmt, va_list al);
    void format(std::string fmt, ...);

   private:
    LogLevel::Level m_level;       // 日志消息级别
    const char* m_loggername;      //日志名称
    const char* m_file = nullptr;  //文件名
    int32_t m_line;                //行号
    uint32_t m_threadId;           //线程号
    uint64_t m_time;               //时间

    std::stringstream m_ss;    //内容字符串流
};

class LogEventWrap {
   public:
    LogEventWrap(std::shared_ptr<Logger> logger,
                 std::shared_ptr<LogEvent> event);
    std::stringstream& getSS();
    ~LogEventWrap();

   private:
    std::shared_ptr<LogEvent> m_event;
    std::shared_ptr<Logger> m_logger;
};
}  // namespace mytinywebserver