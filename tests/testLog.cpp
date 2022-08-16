#include <unistd.h>

#include <iostream>

#include "../src/log/Logger.h"
#include "../src/log/LogAppender.h"
#include "../src/log/LogFormatter.h"
#include "../src/log/LogUtils.h"

using namespace mytinywebserver;

int main() {
    // std::shared_ptr<Logger> logger(new Logger("./log.txt"));
    // logger->setLevel(LogLevel::Level::DEBUG);

    // std::shared_ptr<LogAppender> stdout_appender(
    //     new StdoutLogAppender());
    // std::shared_ptr<LogAppender> file_appender(
    //     new FileLogAppender("./log.txt"));

    // std::shared_ptr<LogFormatter> fmt(new LogFormatter("%d%T%p%T%m%n"));
    // file_appender->setFormatter(fmt);
    // file_appender->setLevel(LogLevel::Level::DEBUG);

    // logger->addAppender(stdout_appender);
    // logger->addAppender(file_appender);

    // auto l = LoggerManager::getInstance()->getLogger("cx");

    LOG_INFO << "cx";
    LOG_INFO << "chenxiiix";

    return 0;
}