#include <unistd.h>

#include <ctime>
#include <iostream>

#include "../src/log/LogManager.h"

using namespace mytinywebserver;

int main() {
    int times = 5;
    double cost_average = 0;
    for (int i = 0; i < times; ++i) {
        clock_t begin, end;
        begin = clock();
        for (int i = 0; i < 100000; i++) {
            LOG_INFO << "cx";
        }
        end = clock();
        int per_cost = (double(end - begin) / CLOCKS_PER_SEC * 1000);
        std::cout << per_cost << std::endl;
        cost_average += per_cost;
    }
    cost_average /= times;
    std::cout << "cost time " << cost_average << " ms" << std::endl;
    return 0;
}