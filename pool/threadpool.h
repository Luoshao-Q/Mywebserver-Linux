#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <atomic>

class ThreadPool {
public:
    ThreadPool(size_t numThreads);
    ~ThreadPool();

    template<typename F, typename... Args>
    void enqueue(F&& f, Args&&... args);

    void stop();

private:
    void worker();

    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_tasks;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::atomic<bool> m_stop;
};

template<typename F, typename... Args>
void ThreadPool::enqueue(F&& f, Args&&... args) {
    auto task = [f = std::forward<F>(f), ...args = std::forward<Args>(args)]() mutable {
        f(std::forward<Args>(args)...);
    };
    std::function<void()> wrapped = std::move(task);

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_stop) return;
        m_tasks.push(std::move(wrapped));
    }
    m_cv.notify_one();
}

#endif