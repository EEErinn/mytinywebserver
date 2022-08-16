#pragma once

#include <algorithm>
#include <string>
#include <vector>

namespace mytinywebserver {

/**
 * @brief
 * prependable 0 readIndex
 * readable readIndex writeIndex
 * writeable writeIndex size
 */
class Buffer {
   private:
    static const int m_cheapPrepend = 8;    // prependable初始大小
    static const int m_initialSize = 1024;  // writeable初始大小

    static const int extrabufSize = 65536;

   public:
    Buffer(size_t initialSize = m_initialSize);
    ~Buffer();

    int readFd(int fd, int* saveErrno);       // 从fd中读取数据
    int writeFd(int fd, int* saveErrno);      // 往fd中写入数据
    void append(const char* begin, int len);  // 往buffer写入数据
    void append(const std::string& str) { append(str.data(), str.size()); }
    void ensureWritableBytes(size_t len);
    void makeSpace(size_t len);
    const char* findCRLF() const;

    void moveReadIndex(int n);  // 移动读索引
    void moveToFront();  // 此时buffer没数据，读写索引移动到最初位置

    // 把onMessage函数上报的buffer数据,转成string类型的数据返回，并修改buffer读写索引
    std::string retrieveAllAsString();
    std::string retrieveAsString(size_t len);
    void retrieveUntil(const char* end);

    size_t prependableSize() const;
    size_t readableSize() const;
    size_t writeableSize() const;
    const char* begin() const;
    const char* beginRead() const;
    const char* beginWrite() const;
    char* begin();
    char* beginWrite();

   private:
    std::vector<char> m_data;  // 动态内存
    int m_readIndex;           // 从该下标处读
    int m_writeIndex;          // 从该下标处写

    static const char kCRLF[];
};

}  // namespace mytinywebserver