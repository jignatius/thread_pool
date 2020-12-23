//
// Created by jacob on 23/12/2020.
//

#ifndef THREADPOOL_PACKAGEDPOOL_H
#define THREADPOOL_PACKAGEDPOOL_H

#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <functional>
#include <condition_variable>
#include <future>
#include <type_traits>
#include <memory>

//Wrapper for packages task so that we can
struct Task {
    template<class F,
            class dF=std::decay_t<F>,
            class=decltype( std::declval<dF&>()() )
    >
    Task(F&& f ):
            ptr(
                    new dF(std::forward<F>(f)),
                    [](void* ptr){ delete static_cast<dF*>(ptr); }
            ),
            invoke([](void*ptr){
                (*static_cast<dF*>(ptr))();  //lambda to invoke call operator on packaged task
            })
    {}
    void operator()() const{
        invoke( ptr.get() );  //call lambda
    }
    Task(Task&&) = default;
    Task&operator=(Task&&) = default;

    Task()=default;
    ~Task()=default;

    explicit operator bool()const { return static_cast<bool>(ptr); }

private:
    std::unique_ptr<void, void(*)(void*)> ptr;  //unique ptr for storing packaged task
    void(*invoke)(void*) = nullptr;  //pointer to packaged task
};

class PackagedPool {
public:
    PackagedPool(): mThreads{}, mTerminate{false}, mQueueMutex{}
    {
        const int num = std::thread::hardware_concurrency();
        for(int i = 0; i < num; i++)
        {
            mThreads.push_back(std::thread(&PackagedPool::Run, this));
        }
    }

    ~PackagedPool()
    {
        for (auto &th : mThreads)
        {
            if (th.joinable())
            {
                th.join();
            }
        }
    }

    void Shutdown()
    {
        {
            std::lock_guard<std::mutex> lk(mQueueMutex);
            mTerminate = true;
        }
        mCondition.notify_all();
    }

    template<typename Fn, typename... Args, typename R=std::result_of_t<Fn(Args...)>>
    std::future<R> Add(Fn&& fn, Args&&... args)
    {
        std::future<R> r;
        {
            std::packaged_task<R()> task(std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...));
            r = task.get_future();
            std::lock_guard<std::mutex> lock(mQueueMutex);
            mQueue.push(std::move(task));
        }
        mCondition.notify_all();
        return r;
    }

protected:
    void Run()
    {
        while(true)
        {
            {
                std::unique_lock<std::mutex> lock(mQueueMutex);
                mCondition.wait(lock, [this]{ return !mQueue.empty() || mTerminate; });

                if (mTerminate)
                {
                    break;
                }

                auto task = std::move(mQueue.front());
                mQueue.pop();
                lock.unlock();

                task();
            }
        }
    }

private:
    std::vector<std::thread> mThreads;
    bool mTerminate;
    std::mutex mQueueMutex;
    std::queue<Task> mQueue;
    std::condition_variable mCondition;
};

#endif //THREADPOOL_PACKAGEDPOOL_H
