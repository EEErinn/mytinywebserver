#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Logger.h"
namespace mytinywebserver {

class Logger;
class LogEvent;
// class LogLevel;

// 日志格式器
class LogFormatter {
   public:
    using ptr = std::shared_ptr<LogFormatter>;
    LogFormatter(const std::string& pattern);
    // 将event转换为字符串
    std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level,
                       std::shared_ptr<LogEvent> event);

   public:
    class FormatItem {
       public:
        typedef std::shared_ptr<FormatItem> ptr;
        virtual ~FormatItem() {}
        virtual void format(std::ostream& os, std::shared_ptr<Logger> logger,
                            LogLevel::Level level,
                            std::shared_ptr<LogEvent> event) = 0;
    };

    void init();  // pattern 解析
    bool isError() const { return m_error; }
    const std::string getPattern() const { return m_pattern; }

   private:
    std::string m_pattern;  // format结构
    std::vector<FormatItem::ptr> m_items;
    bool m_error = false;
};
}  // namespace mytinywebserver