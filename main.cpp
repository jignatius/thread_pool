#include <iostream>
#include <chrono>
#include "threadpool.h"
#include "packagedpool.h"

class MyClass {
public:
    void memfunc(std::string const& prefix, int i) const {
        std::cout << prefix << " " << i << "\n";
    }
};

int main() {
    PackagedPool pool;
    auto add = [](int x, int y){ return x + y; };
    auto add2 = [](int x, int y, int z){ return x + y + z; };

    auto fut = pool.Add(add, 1, 3);
    auto fut2 = pool.Add(add, 10, 10);
    auto fut3 = pool.Add(add, 30, 3);
    auto fut4 = pool.Add(add2, 30, 30,30);

    auto res1 = fut.get();
    std::cout << res1 << std::endl;
    auto res2 = fut2.get();
    std::cout << res2 << std::endl;
    auto res3 = fut3.get();
    std::cout << res3 << std::endl;
    auto res4 = fut4.get();
    std::cout << res4 << std::endl;

    MyClass obj;
    ThreadPool threadPool;
    threadPool.Add([](std::string msg){ std::cout << msg << std::endl; }, std::string("hello world"));
    threadPool.Add([](int num){ std::cout << num << std::endl; }, 888);
    threadPool.Add([](int a, int b){ std::cout << a << "*" << b << " = " << a * b << std::endl; }, 100, 2);
    threadPool.Add(&MyClass::memfunc, &obj, "year", 2020);

    std::this_thread::sleep_for(std::chrono::seconds(1));
    pool.Shutdown();
    threadPool.Shutdown();

    return 0;
}