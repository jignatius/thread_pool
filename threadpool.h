//
// Created by jacob on 23/12/2020.
//

#ifndef THREADPOOL_THREADPOOL_H
#define THREADPOOL_THREADPOOL_H

#include <thread>
#include <vector>
#include <mutex>
#include <utility>
#include <queue>
#include <functional>
#include <condition_variable>


class ThreadPool {
public:
    ThreadPool(): mThreads{}, mTerminate{false}, mQueueMutex{}
    {
        const int num = std::thread::hardware_concurrency();
        for(int i = 0; i < num; i++)
        {
            mThreads.push_back(std::thread(&ThreadPool::Run, this));
        }
    }

    ~ThreadPool()
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

    template<typename Fn, typename... Args>
    void Add(Fn&& fn, Args&&... args)
    {
        {
            std::function<void()> job = std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...);
            std::lock_guard<std::mutex> lock(mQueueMutex);
            mQueue.push(job);
        }
        mCondition.notify_all();
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

                auto job = mQueue.front();
                mQueue.pop();
                lock.unlock();

                job();
            }
        }
    }

private:
    std::vector<std::thread> mThreads;
    bool mTerminate;
    std::mutex mQueueMutex;
    std::queue<std::function<void()>> mQueue;
    std::condition_variable mCondition;
};


#endif //THREADPOOL_THREADPOOL_H
