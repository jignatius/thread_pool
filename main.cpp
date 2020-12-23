#include <iostream>
#include <chrono>
#include "threadpool.h"

int main() {
    ThreadPool threadPool;
    threadPool.Add([](std::string msg){ std::cout << msg << std::endl; }, std::string("hello world"));
    threadPool.Add([](int num){ std::cout << num << std::endl; }, 888);
    threadPool.Add([](int a, int b){ std::cout << a * b << std::endl; }, 100, 2);

    std::this_thread::sleep_for(std::chrono::seconds(1));
    threadPool.Shutdown();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    return 0;
}