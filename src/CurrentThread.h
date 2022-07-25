// Copyright©2022 Erin
#pragma once

#include <sys/syscall.h>
#include <unistd.h>

namespace mytinywebserver {
namespace CurrentThread {

extern __thread int t_cachedTid;  // 保存tid缓存 因为系统调用非常耗时
void cacheTid();

inline pid_t tid() {
    // __builtin_expect 是一种底层优化
    // 进入if时如果还未获取tid，通过cacheTid()系统调用获取tid
    if (__builtin_expect(t_cachedTid == 0, 0)) {
        cacheTid();
    }
    return t_cachedTid;
}

}  // namespace CurrentThread
}  // namespace mytinywebserver