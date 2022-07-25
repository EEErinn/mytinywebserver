// Copyright©2022 Erin
#pragma once

class noncopyable {
   public:
    noncopyable() = default;
    ~noncopyable() = default;

   private:
    noncopyable(const noncopyable&) = delete;
    // noncopyable(noncopyable&&) = delete; // 可以移动构造
    noncopyable& operator=(const noncopyable&) = delete;
};
