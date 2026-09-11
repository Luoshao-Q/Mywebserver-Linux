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

    std::vector<std::thread> m_workers;  // 工作线程池
    std::queue<std::function<void()>> m_tasks;  // 任务队列
    std::mutex m_mutex;  // 保护任务队列的锁
    std::condition_variable m_cv;  // 线程等待/唤醒条件变量
    std::atomic<bool> m_stop;  // 停止标志（原子操作，线程安全）
};

template<typename F, typename... Args>
void ThreadPool::enqueue(F&& f, Args&&... args) {
    auto task = [f = std::forward<F>(f), ...args = std::forward<Args>(args)]() mutable {
        f(std::forward<Args>(args)...);
    };
    std::function<void()> wrapped = std::move(task);

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_stop) return;  //是为了防止“线程池正在销毁时，还有人往里面塞任务”。
        m_tasks.push(std::move(wrapped));
    }
    m_cv.notify_one();
}

#endif