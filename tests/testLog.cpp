#include <unistd.h>

#include <ctime>
#include <iostream>

#include "../src/log/LogManager.h"

using namespace mytinywebserver;

int main() {
    clock_t begin, end;
    begin = clock();
    for (int i = 0; i < 10000; i++) {
        LOG_INFO << "cx";
    }
    end = clock();
    std::cout << "cost time " << double(end - begin) / CLOCKS_PER_SEC * 1000
              << " ms" << std::endl;
    return 0;
}