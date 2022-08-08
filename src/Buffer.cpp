#include "Buffer.h"

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/uio.h>

namespace mytinywebserver {
Buffer::Buffer(size_t initialSize)
    : m_data(m_cheapPrepend + initialSize),
      m_readIndex(m_cheapPrepend),
      m_writeIndex(m_cheapPrepend) {}

Buffer::~Buffer() {}

int Buffer::readFd(int fd, int* saveErrno) {
    char extrabuf[extrabufSize];
    struct iovec vec[2];
    auto writeable = writeableSize();
    vec[0].iov_base = beginWrite();
    vec[0].iov_len = writeable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);
    ssize_t n = readv(fd, vec, 2);
    if (n < 0) {
        // FIXME: LOG_FATAL
        *saveErrno = errno;
    } else if (static_cast<size_t>(n) <= writeable) {
        m_writeIndex += n;
    } else {
        // 将extra的数据放入m_data, 移动m_writeIndex指针
        // FIXME: 考虑扩容，数据移动
        m_writeIndex += m_data.size();
        append(extrabuf, strlen(extrabuf));
    }
    return n;
}

void Buffer::append(const char* begin, int len) {
    m_data.insert(m_data.begin() + m_writeIndex, begin, begin + len);
    m_writeIndex += len;
}

int Buffer::writeFd(int fd, int* saveErrno) {
    auto n = ::send(fd, beginRead(), readableSize(), 0);
    if (n < 0) {
        // FIXME: LOG_FATAL
        *saveErrno = errno;
    }
    return n;
}

void Buffer::moveReadIndex(int n) {
    assert(static_cast<size_t>(n) <= readableSize());
    if (static_cast<size_t>(n) <= readableSize()) {
        m_readIndex += n;
    } else {
        moveToFront();
    }
}
void Buffer::moveToFront() {
    m_readIndex = m_cheapPrepend;
    m_writeIndex = m_cheapPrepend;
}

size_t Buffer::prependableSize() { return m_readIndex; }
size_t Buffer::readableSize() { return m_writeIndex - m_readIndex; }
size_t Buffer::writeableSize() { return m_data.size() - m_writeIndex; }

char* Buffer::begin() { return &*m_data.begin(); }
char* Buffer::beginRead() { return begin() + m_readIndex; }
char* Buffer::beginWrite() { return begin() + m_writeIndex; }

}  // namespace mytinywebserver