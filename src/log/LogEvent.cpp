#include "LogEvent.h"

#include "Logger.h"
namespace mytinywebserver {

LogEvent::LogEvent(LogLevel::Level level, const char* loggername,
                   const char* file, int32_t line, uint32_t thread_id,
                   uint64_t time)
    : m_level(level),
      m_loggername(loggername),
      m_file(file),
      m_line(line),
      m_threadId(thread_id),
      m_time(time) {}

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
                           std::shared_ptr<LogEvent> event)
    : m_event(event), m_logger(logger) {}

std::stringstream& LogEventWrap::getSS() { return m_event->getSS(); }

LogEventWrap::~LogEventWrap() { m_logger->log(m_event); }

}  // namespace mytinywebserver