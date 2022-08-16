#include "LogEvent.h"

#include "Logger.h"
namespace mytinywebserver {

LogEvent::LogEvent(const char* file, int32_t line, uint32_t elapse,
                   uint32_t thread_id, uint32_t fiber_id, uint64_t time,
                   const std::string& thread_name)
    : m_file(file),
      m_line(line),
      m_elapse(elapse),
      m_threadId(thread_id),
      m_fiberId(fiber_id),
      m_time(time),
      m_threadName(thread_name) {}

void LogEvent::format(std::string fmt, ...) {
    va_list valst;
    va_start(valst, fmt);
    format(fmt, valst);
    va_end(valst);
}

void LogEvent::format(std::string fmt, va_list al) {
    char* buf = nullptr;
    int len = vasprintf(&buf, fmt.c_str(), al);  // note 加上最大size的限制
    if (len != -1) {
        m_ss << std::string(buf, len);
        free(buf);
    }
}

LogEventWrap::LogEventWrap(std::shared_ptr<Logger> logger,
                           LogLevel::Level level,
                           std::shared_ptr<LogEvent> event)
    : m_event(event), m_logger(logger), m_level(level) {}

std::stringstream& LogEventWrap::getSS() { return m_event->getSS(); }

LogEventWrap::~LogEventWrap() { m_logger->log(m_level, m_event); }

}  // namespace mytinywebserver