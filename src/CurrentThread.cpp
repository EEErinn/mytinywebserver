// Copyright©2022 Erin
#include "CurrentThread.h"

namespace mytinywebserver {
namespace CurrentThread {

__thread int t_cachedTid = 0;

void cacheTid() {
    if (t_cachedTid == 0) {
        t_cachedTid = ::syscall(SYS_gettid);
    }
}

}  // namespace CurrentThread
}  // namespace mytinywebserver