// Copyright©2022 Erin
#include "Timestamp.h"

#include <string.h>
#include <time.h>

#include "log/LogManager.h"

namespace mytinywebserver {

Timestamp::Timestamp() : m_ms(0) {}
Timestamp::Timestamp(int64_t ms) : m_ms(ms) {}

Timestamp Timestamp::now() { return Timestamp(time(NULL)); }

bool Timestamp::operator<(const Timestamp& v) { return m_ms < v.getMs(); }

// FIXME: 用gettimeofday优化 https://github.com/LeechanX/Ring-Log
std::string Timestamp::toString() const {
    char buf[100] = {'\0'};
    struct tm* t = localtime(&m_ms);
    snprintf(buf, sizeof(buf), "%d %d %d %d %d %d", t->tm_year + 1900,
             t->tm_mon + 1, t->tm_mday + 1, t->tm_hour + 1, t->tm_min + 1,
             t->tm_sec + 1);
    return buf;
}

}  // namespace mytinywebserver