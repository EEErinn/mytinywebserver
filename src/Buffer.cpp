#include "Buffer.h"

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/uio.h>

#include "log/LogUtils.h"

namespace mytinywebserver {
const char Buffer::kCRLF[] = "\r\n";

Buffer::Buffer(size_t initialSize)
    : m_data(m_cheapPrepend + initialSize),
      m_readIndex(m_cheapPrepend),
      m_writeIndex(m_cheapPrepend) {}

Buffer::~Buffer() {}

int Buffer::readFd(int fd, int* saveErrno) {
    char extrabuf[extrabufSize] = {'0'};
    struct iovec vec[2];
    auto writable = writeableSize();
    vec[0].iov_base = beginWrite();
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);
    const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
    LOG_DEBUG << fd << " " << writable << " " << sizeof(extrabuf) << iovcnt;
    ssize_t n = readv(fd, vec, iovcnt);
    if (n < 0) {
        *saveErrno = errno;
    } else if (static_cast<size_t>(n) <= writable) {
        m_writeIndex += n;
    } else {
        LOG_DEBUG << "extra data";
        // 将extra的数据放入m_data, 移动m_writeIndex指针
        m_writeIndex = m_data.size();
        append(extrabuf, n - writable);
    }
    return n;
}

void Buffer::append(const char* begin, int len) {
    ensureWritableBytes(len);                     // 判断要不要扩容
    std::copy(begin, begin + len, beginWrite());  // 赋值
    // m_data.insert(m_data.begin() + m_writeIndex, begin, begin + len);
    m_writeIndex += len;  // 修改写索引
}

// 是否扩容
void Buffer::ensureWritableBytes(size_t len) {
    if (writeableSize() < len) {
        makeSpace(len);
    }
    assert(writeableSize() >= len);
}

void Buffer::makeSpace(size_t len) {
    if (writeableSize() + prependableSize() < len + m_cheapPrepend) {
        // FIXME: move readable data
        m_data.resize(m_writeIndex + len);
    } else {
        // move readable data to the front, make space inside buffer
        assert(m_cheapPrepend < m_readIndex);
        size_t readable = readableSize();
        std::copy(begin() + m_readIndex, begin() + m_writeIndex,
                  begin() + m_cheapPrepend);
        m_readIndex = m_cheapPrepend;
        m_writeIndex = m_readIndex + readable;
        assert(readable == readableSize());
    }
}

int Buffer::writeFd(int fd, int* saveErrno) {
    auto n = ::send(fd, beginRead(), readableSize(), 0);
    if (n < 0) {
        // FIXME: LOG_FATAL
        *saveErrno = errno;
    }
    return n;
}

const char* Buffer::findCRLF() const {
    // FIXME: replace with memmem()?
    const char* crlf = std::search(beginRead(), beginWrite(), kCRLF, kCRLF + 2);
    return crlf == beginWrite() ? NULL : crlf;
}

void Buffer::moveReadIndex(int n) {
    assert(static_cast<size_t>(n) <= readableSize());
    if (static_cast<size_t>(n) < readableSize()) {
        m_readIndex += n;
    } else {
        moveToFront();
    }
}

void Buffer::moveToFront() {
    m_readIndex = m_cheapPrepend;
    m_writeIndex = m_cheapPrepend;
}

std::string Buffer::retrieveAllAsString() {
    return retrieveAsString(readableSize());
}

std::string Buffer::retrieveAsString(size_t len) {
    assert(len <= readableSize());
    std::string result(beginRead(), len);
    moveReadIndex(len);
    return result;
}

void Buffer::retrieveUntil(const char* end) {
    assert(beginRead() <= end);
    assert(end <= beginWrite());
    moveReadIndex(end - beginRead());
}

size_t Buffer::prependableSize() const { return m_readIndex; }
size_t Buffer::readableSize() const { return m_writeIndex - m_readIndex; }
size_t Buffer::writeableSize() const { return m_data.size() - m_writeIndex; }

const char* Buffer::begin() const { return &*m_data.begin(); }
const char* Buffer::beginRead() const { return begin() + m_readIndex; }
const char* Buffer::beginWrite() const { return begin() + m_writeIndex; }
char* Buffer::begin() { return &*m_data.begin(); }
char* Buffer::beginWrite() { return begin() + m_writeIndex; }

}  // namespace mytinywebserver
