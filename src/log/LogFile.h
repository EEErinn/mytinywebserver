#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <string.h>

#include "../noncopyable.h"

namespace mytinywebserver {

namespace detail {
const int kSmallBuffer = 4000;         // 4M
const int kLargeBuffer = 4000 * 1000;  // 4000M
const int kbuffersMaxSize = 25;

template <int SIZE>
class FixedBuffer : noncopyable {
   public:
    FixedBuffer() : cur_(data_) {}

    void append(const std::string& info) { append(info.c_str(), info.size()); }

    void append(const char* /*restrict*/ buf, size_t len) {
        // FIXME: append partially
        if (static_cast<size_t>(avail()) > len) {
            memcpy(cur_, buf, len);
            cur_ += len;
        }
    }

    const char* data() const { return data_; }
    int length() const { return static_cast<int>(cur_ - data_); }

    // write to data_ directly
    char* current() { return cur_; }
    int avail() const { return static_cast<int>(end() - cur_); }
    void add(size_t len) { cur_ += len; }

    void reset() { cur_ = data_; }
    void bzero() { memset(data_, '0', sizeof(data_)); }

    std::string toString() const { return std::string(data_, length()); }

   private:
    const char* end() const { return data_ + sizeof(data_); }

    char data_[SIZE];
    char* cur_;
};
}  // namespace detail

// 封装File api
class AppendFile : noncopyable {
   public:
    AppendFile(const char* filename);
    ~AppendFile();
    void append(const char* logline, size_t len);         // 写入文件
    void flush();                                         // 强制刷新
    off_t writtenBytes() const { return writtenBytes_; }  // 文件内容的大小

   private:
    size_t write(const char* logline, size_t len);

    FILE* fp_;
    char buffer_[64 * 1024];
    off_t writtenBytes_;
};

//
class LogFile : noncopyable {
   public:
    using ptr = std::shared_ptr<LogFile>;
    LogFile(const std::string& basename, off_t rollSize, bool threadSafe = true,
            int flushInterval = 3, int checkEveryN = 1024);
    ~LogFile();

    void append(const char* logline, int len);  //添加日志，判断是否需要roll
    void flush();
    bool rollFile();

   private:
    void append_unlocked(const char* logline, int len);

    static std::string getLogFileName(const std::string& basename, time_t* now);

    const std::string basename_;
    const off_t rollSize_;
    const int flushInterval_;
    const int checkEveryN_;

    int count_;

    std::mutex mutex_;
    time_t startOfPeriod_;
    time_t lastRoll_;
    time_t lastFlush_;
    std::unique_ptr<AppendFile> file_;

    const static int kRollPerSeconds_ = 60 * 60 * 24;
};

}  // namespace mytinywebserver
