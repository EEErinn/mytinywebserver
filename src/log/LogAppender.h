#pragma once
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "LogEvent.h"
#include "LogFile.h"
#include "LogFormatter.h"
#include "Logger.h"

namespace mytinywebserver {

//日志输出地
class LogAppender {
   public:
    using ptr = std::shared_ptr<LogAppender>;
    LogAppender() {}
    virtual ~LogAppender() {}

    virtual void log(std::shared_ptr<Logger> logger,
                     std::shared_ptr<LogEvent> event) = 0;
    virtual void flush() {}
    virtual void setNotifyFunc(const std::function<void()>& notify_func) {}

    LogFormatter::ptr getFormatter() { return m_formatter; }
    void setFormatter(LogFormatter::ptr val) { m_formatter = val; }

   protected:
    LogFormatter::ptr m_formatter;
    std::mutex m_mutex;
};

// 同步输出到控制台
class StdoutLogAppender : public LogAppender {
   public:
    using ptr = std::shared_ptr<StdoutLogAppender>;
    void log(std::shared_ptr<Logger> logger,
             std::shared_ptr<LogEvent> event) override {
        if (event->getLevel() >= logger->getLevel()) {
            std::unique_lock<std::mutex> lock(m_mutex);
            std::cout << m_formatter->format(event);
        }
    }
};

// 同步输出到文件
class SyncFileLogAppender : public LogAppender {
   public:
    using ptr = std::shared_ptr<StdoutLogAppender>;
    explicit SyncFileLogAppender(const std::string& filename)
        : m_filename(filename) {
        if (!reopen()) throw std::exception();
    }
    void log(std::shared_ptr<Logger> logger,
             std::shared_ptr<LogEvent> event) override {
        if (event->getLevel() >= logger->getLevel()) {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_filestream << m_formatter->format(event);
        }
    }
    bool reopen();
    ~SyncFileLogAppender() {
        if (m_filestream) {
            m_filestream.flush();
            m_filestream.close();
        }
    }

   private:
    std::string m_filename;
    std::ofstream m_filestream;
};

// 异步输出到文件
class AyncFileLogAppender : public LogAppender {
   public:
    AyncFileLogAppender(const std::string& filepath, off_t rollSize);
    virtual ~AyncFileLogAppender() {}

    void log(std::shared_ptr<Logger> logger,
             std::shared_ptr<LogEvent> event) override;
    void flush() override;

    void setNotifyFunc(const std::function<void()>& notify_func) override;
    bool empty();

   private:
    using Buffer = mytinywebserver::detail::FixedBuffer<
        mytinywebserver::detail::kLargeBuffer>;
    using BufferVector = std::vector<std::unique_ptr<Buffer>>;
    using BufferPtr = BufferVector::value_type;

    BufferPtr currentBuffer_;  // 当前缓冲
    BufferPtr nextBuffer_;     // 预备缓冲
    BufferVector buffers_;     //待写入文件的已填满的缓冲区

    // 输出缓冲区们
    BufferPtr newBuffer1;
    BufferPtr newBuffer2;
    BufferVector buffersToWrite;

    const std::string filepath_;
    const off_t rollSize_;
    std::unique_ptr<LogFile> file_;
    uint64_t lastTime_;

    // 唤醒日志线程
    std::function<void()> notify_func_;  // 回调函数
};

}  // namespace mytinywebserver
