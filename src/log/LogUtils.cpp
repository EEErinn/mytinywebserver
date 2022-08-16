#include "LogUtils.h"

#include <string.h>

std::shared_ptr<mytinywebserver::Logger> logger_(
    mytinywebserver::Logger::getInstance(std::string("./log.txt")));

__thread char t_errnobuf[512];
namespace mytinywebserver {
const char* strerror_tl(int savedErrno) {
    return strerror_r(savedErrno, t_errnobuf, sizeof t_errnobuf);
}
}  // namespace mytinywebserver
