#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t n) {
    for (size_t i = 0; i < n; ++i)
        workers_.emplace_back([this]{
            for (;;) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lk(mtx_);
                    cv_.wait(lk, [this]{ return stop_ || !tasks_.empty(); });
                    if (stop_ && tasks_.empty()) return;
                    task = std::move(tasks_.front()); tasks_.pop();
                    ++active_;
                }
                try { task(); } catch (...) {}
                {
                    std::lock_guard<std::mutex> lk(mtx_);
                    --active_;
                    if (tasks_.empty() && active_ == 0) cvDone_.notify_all();
                }
            }
        });
}

ThreadPool::~ThreadPool() {
    { std::lock_guard<std::mutex> lk(mtx_); stop_ = true; }
    cv_.notify_all();
    for (auto& t : workers_) if (t.joinable()) t.join();
}

void ThreadPool::enqueue(std::function<void()> task) {
    { std::lock_guard<std::mutex> lk(mtx_); tasks_.push(std::move(task)); }
    cv_.notify_one();
}

void ThreadPool::wait() {
    std::unique_lock<std::mutex> lk(mtx_);
    cvDone_.wait(lk, [this]{ return tasks_.empty() && active_ == 0; });
}
