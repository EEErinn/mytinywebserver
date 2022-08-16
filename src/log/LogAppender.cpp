#include "LogAppender.h"

namespace mytinywebserver {
bool FileLogAppender::reopen() {
    if (m_filestream) {
        m_filestream.close();
    }
    m_filestream.open(m_filename, std::ios::app);
    return !!m_filestream;  //让大于0 输出1 小于0 输出
}
}  // namespace mytinywebserver