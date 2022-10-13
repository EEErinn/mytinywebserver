#include "LogAppender.h"

#include <assert.h>

#include "../Timestamp.h"

namespace mytinywebserver {
bool SyncFileLogAppender::reopen() {
    if (m_filestream) {
        m_filestream.close();
    }
    m_filestream.open(m_filename, std::ios::app);
    return !!m_filestream;  //让大于0 输出1 小于0 输出
}

AyncFileLogAppender::AyncFileLogAppender(const std::string &filepath,
                                         off_t rollSize)
    : currentBuffer_(std::make_unique<Buffer>()),
      nextBuffer_(std::make_unique<Buffer>()),
      newBuffer1(std::make_unique<Buffer>()),
      newBuffer2(std::make_unique<Buffer>()),
      filepath_(filepath),
      rollSize_(rollSize) {
    newBuffer1->bzero();
    newBuffer2->bzero();
    buffersToWrite.reserve(16);
    file_ = std::make_unique<LogFile>(filepath_, rollSize_, false);
}

void AyncFileLogAppender::setNotifyFunc(
    const std::function<void()> &notify_func) {
    notify_func_ = notify_func;
}

// 其他线程会调用，保证线程安全
void AyncFileLogAppender::log(std::shared_ptr<Logger> logger,
                              std::shared_ptr<LogEvent> event) {
    if (event->getLevel() >= logger->getLevel()) {
        std::string info = m_formatter->format(event);
        int len = info.size();

        std::unique_lock<std::mutex> lock(m_mutex);
        if (currentBuffer_->avail() > len) {
            currentBuffer_->append(info);
        } else {
            buffers_.push_back(std::move(currentBuffer_));

            if (nextBuffer_) {
                currentBuffer_ = std::move(nextBuffer_);
            } else {
                currentBuffer_.reset(new Buffer);  // Rarely happens
            }
            currentBuffer_->append(info);

            // 唤醒日志线程
            if (notify_func_) {
                notify_func_();
            }
        }
    }
}

void AyncFileLogAppender::flush() {
    assert(newBuffer1 && newBuffer1->length() == 0);
    assert(newBuffer2 && newBuffer2->length() == 0);
    assert(buffersToWrite.empty());

    {
        std::unique_lock<std::mutex> lock(m_mutex);
        buffers_.push_back(std::move(currentBuffer_));
        currentBuffer_ = std::move(newBuffer1);
        buffersToWrite.swap(buffers_);
        if (!nextBuffer_) {
            nextBuffer_ = std::move(newBuffer2);
        }
    }

    assert(!buffersToWrite.empty());

    // 如果突然写入太多数据
    if (buffersToWrite.size() > detail::kbuffersMaxSize) {
        int leftSize = detail::kbuffersMaxSize;
        char buf[256];
        snprintf(buf, sizeof(buf),
                 "Dropped log messages at %s, %zd larger buffers\n",
                 Timestamp::now().toString().c_str(),
                 buffersToWrite.size() - leftSize);
        fputs(buf, stderr);
        file_->append(buf, static_cast<int>(strlen(buf)));
        // 保留一部分数据，丢弃多的，防止磁盘被冲垮
        buffersToWrite.erase(buffersToWrite.begin() + leftSize,
                             buffersToWrite.end());
    }

    for (const auto &buffer : buffersToWrite) {
        // FIXME: use unbuffered stdio FILE ? or use ::writev ?
        file_->append(buffer->data(), buffer->length());
    }

    if (buffersToWrite.size() > 2) {
        // drop non-bzero-ed buffers, avoid trashing
        buffersToWrite.resize(2);
    }

    if (!newBuffer1) {
        assert(!buffersToWrite.empty());
        newBuffer1 = std::move(buffersToWrite.back());
        buffersToWrite.pop_back();
        newBuffer1->reset();
    }

    if (!newBuffer2) {
        assert(!buffersToWrite.empty());
        newBuffer2 = std::move(buffersToWrite.back());
        buffersToWrite.pop_back();
        newBuffer2->reset();
    }

    buffersToWrite.clear();
    file_->flush();
}

}  // namespace mytinywebserver