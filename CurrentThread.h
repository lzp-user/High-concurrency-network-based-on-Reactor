#pragma once

#include <unistd.h>
#include <sys/syscall.h>

namespace CurrentThread
{
    extern __thread int t_cachedTid; // 当前线程id

    void cacheTid(); // 获取当前线程id,通过linux系统调用

    inline int tid() // 返回当前线程id
    {
        if (__builtin_expect(t_cachedTid == 0, 0))
        {
            cacheTid();
        }
        return t_cachedTid;
    }
}