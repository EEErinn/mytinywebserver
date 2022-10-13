#include "LogManager.h"

#include <string.h>

bool log_open = true;
bool isAsync = false;

static std::shared_ptr<mytinywebserver::Logger> getLogger() {
    if (log_open) {
        if (isAsync) {
            return mytinywebserver::LoggerManager::getAsyncFileRoot();
        } else {
            return mytinywebserver::LoggerManager::getSyncFileRoot();
        }
    }
    return nullptr;
}

std::shared_ptr<mytinywebserver::Logger> logger_ = getLogger();