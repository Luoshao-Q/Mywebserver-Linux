#include "threadpool.h"

ThreadPool::ThreadPool(size_t numThreads) : m_stop(false) {
    for (size_t i = 0; i < numThreads; ++i) {
        m_workers.emplace_back([this] { worker(); }); 
        //因为 worker 是成员函数，不是普通函数。在 C++ 里，成员函数不能像普通函数那样直接使用
    }
}

ThreadPool::~ThreadPool() {
    stop();
}

void ThreadPool::stop() {
    m_stop = true;
    m_cv.notify_all();
    for (auto& t : m_workers) {
        if (t.joinable()) {
            t.join();
        }
    }
}

void ThreadPool::worker() {
    while (!m_stop) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this] { return !m_tasks.empty() || m_stop; });
            if (m_stop && m_tasks.empty()) return;
            task = std::move(m_tasks.front());
            m_tasks.pop();
        }
        task();
    }
}