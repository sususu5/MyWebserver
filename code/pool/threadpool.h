#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <thread>
#include <assert.h>

class ThreadPool {
public:
    ThreadPool() = default;
    ThreadPool(ThreadPool&&) = default;
    // Use make_shared instead of new to avoid memory fragmentation
    explicit ThreadPool(int threadCount = 8): pool_(std::make_shared<Pool>()) {
        assert(threadCount > 0);
        for (int i = 0; i < threadCount; i++) {
            // Use detach to avoid joining the threads
            std::thread([this]() {
                std::unique_lock<std::mutex> locker(pool_->mtx_);
                while (true) {
                    if (!pool_->tasks_.empty()) {
                        auto task = std::move(pool_->tasks_.front());
                        pool_->tasks_.pop();
                        // The lock is only used to protect the queue, so it can be released before executing the task
                        locker.unlock();
                        task();
                        // Lock again after executing the task
                        locker.lock();
                    } else if (pool_->isClosed) {
                        break;
                    } else {
                        pool_->cond_.wait(locker);
                    }
                }
            }).detach();
        }
    }

    ~ThreadPool() {
        if (pool_) {
            std::unique_lock<std::mutex> locker(pool_->mtx_);
            pool_->isClosed = true;
        }
        // Notify all threads to exit
        pool_->cond_.notify_all();
    }

    template<typename T>
    // Universal reference is used to accept both lvalue and rvalue
    void AddTask(T&& task) {
        std::unique_lock<std::mutex> locker(pool_->mtx_);
        // The emplace method constructs the task object in queue directly
        // Perfect forwarding is used to avoid unnecessary copy and checking of the task type
        pool_->tasks_.emplace(std::forward<T>(task));
        // Notify one thread to execute the task
        pool_->cond_.notify_one();
    }

private:
    struct Pool {
        std::mutex mtx_;
        std::condition_variable cond_;
        bool isClosed;
        std::queue<std::function<void()>> tasks_;
    };
    std::shared_ptr<Pool> pool_;
};

#endif 