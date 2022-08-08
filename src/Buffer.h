#pragma once

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

    int readFd(int fd, int* saveErrno);   // 从fd中读取数据
    int writeFd(int fd, int* saveErrno);  // 往fd中写入数据
    void append(const char* begin, int len);    // 往buffer写入数据

    void moveReadIndex(int n);  // 移动读索引
    void moveToFront();  // 此时buffer没数据，读写索引移动到最初位置

    size_t prependableSize();
    size_t readableSize();
    size_t writeableSize();

    char* begin();
    char* beginRead();
    char* beginWrite();

   private:
    std::vector<char> m_data;  // 动态内存
    int m_readIndex;           // 从该下标处读
    int m_writeIndex;          // 从该下标处写
};

}  // namespace mytinywebserver