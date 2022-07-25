// Copyright©2022 Erin
#pragma once
#include <string>

namespace mytinywebserver {

class Timestamp {
   public:
    Timestamp();
    explicit Timestamp(int64_t ms);

    static Timestamp now();
    std::string toString() const;

   private:
    int64_t m_ms;
};

}  // namespace mytinywebserver
