// Copyright©2022 Erin
#include "Timestamp.h"

#include <string.h>
#include <time.h>

namespace mytinywebserver {

Timestamp::Timestamp() : m_ms(0) {}
Timestamp::Timestamp(int64_t ms) : m_ms(ms) {}

Timestamp Timestamp::now() { return Timestamp(time(NULL)); }

std::string Timestamp::toString() const {
    char buf[20] = {'\0'};
    struct tm* time = nullptr;
    localtime_r(&m_ms, time);
    snprintf(buf, sizeof(buf), "%d %d %d %d %d %d",
             time->tm_year + 1900, time->tm_mon + 1, time->tm_mday + 1,
             time->tm_hour + 1, time->tm_min + 1, time->tm_sec + 1);
    return buf;
}

}  // namespace mytinywebser