#pragma once

#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>

class ThreadPool {
private:
    std::deque<std::function<void()>> tasks;

    std::vector<std::thread> threads;
    std::mutex mutex;
    std::condition_variable cv;
    bool running = false;

public:
    ThreadPool(int nThreads){
        for(int i = 0; i < nThreads; i++){
            threads.emplace_back(
                [this]
                {
                    while(true){
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(mutex);
                            cv.wait(lock, [this]{ return !running || !tasks.empty(); });
                            if(!running && tasks.empty()) return;
                            task = std::move(tasks.front());
                            tasks.pop_front();
                        }
                        task();
                    }
                }
            );
        }
        {
            std::unique_lock<std::mutex> lock(mutex);
            running = true;
        }
    };

    ~ThreadPool(){
        if(running){
            stop();
        }
    };

    void stop(){
        {
            std::unique_lock<std::mutex> lock(mutex);
            running = false;
        }
        cv.notify_all();
        for(auto& thread : threads){
            thread.join();
        }
    };

    template<typename Func, typename... Args>
    auto addTask(Func&& func, Args&&... args){
        using return_type = typename std::invoke_result<Func, Args...>::type;
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
        );
        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(mutex);
            if(!running) 
                throw std::runtime_error("ThreadPool is stopped");
            this->tasks.emplace_back([task](){ (*task)(); });
        }
        cv.notify_one();
        return res;
    };

    template<typename Func, typename... Args>
    auto addTaskWithHighPriority(Func&& func, Args&&... args){
        using return_type = typename std::invoke_result<Func, Args...>::type;
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
        );
        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(mutex);
            if(!running) 
                throw std::runtime_error("ThreadPool is stopped");
            this->tasks.emplace_front([task](){ (*task)(); });
        }
        cv.notify_one();
        return res;
    };
};