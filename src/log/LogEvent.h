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
    LogEvent(const char* file, int32_t m_line, uint32_t elapse,
             uint32_t thread_id, uint32_t fiber_id, uint64_t time,
             const std::string& thread_name);

    // getter
    const char* getFile() const { return m_file; }
    int32_t getLine() const { return m_line; }
    uint32_t getThreadId() const { return m_threadId; }
    uint32_t getFiberId() const { return m_fiberId; }
    uint32_t getElapse() const { return m_elapse; }
    uint64_t getTime() const { return m_time; }
    std::string getThreadName() const { return m_threadName; }
    std::string getContent() const { return m_ss.str(); }
    std::stringstream& getSS() { return m_ss; }

    // 格式化写入日志内容
    void format(std::string fmt, va_list al);
    void format(std::string fmt, ...);

   private:
    //文件名
    const char* m_file = nullptr;
    //行号
    int32_t m_line;
    //程序启动开始到现在的毫秒
    uint32_t m_elapse;
    //线程号
    uint32_t m_threadId;
    //协程号
    uint32_t m_fiberId;
    //时间
    uint64_t m_time;
    // 线程名称
    std::string m_threadName;
    //内容字符串流
    std::stringstream m_ss;
};

class LogEventWrap {
   public:
    LogEventWrap(std::shared_ptr<Logger> logger, LogLevel::Level level,
                 std::shared_ptr<LogEvent> event);
    std::stringstream& getSS();
    ~LogEventWrap();

   private:
    std::shared_ptr<LogEvent> m_event;
    std::shared_ptr<Logger> m_logger;
    LogLevel::Level m_level;
};
}  // namespace mytinywebserver