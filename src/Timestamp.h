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
    bool operator<(const Timestamp&);
    int getMs() const { return m_ms; }

   private:
    int64_t m_ms;
};

inline Timestamp addTime(Timestamp timestamp, double seconds) {
    int64_t delta = static_cast<int64_t>(seconds);
    return Timestamp(timestamp.getMs() + delta);
}

}  // namespace mytinywebserver
