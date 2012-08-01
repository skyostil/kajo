// Copyright (C) 2012 Sami Kyöstilä

#include <queue>
#include <mutex>
#include <condition_variable>
#include <tuple>
#include <chrono>

template <typename T>
class Queue
{
public:
    Queue():
        m_done(false)
    {
    }

    void push(const T& item)
    {
        std::unique_lock<std::mutex> lock(m_lock);
        m_queue.push(item);
        m_signal.notify_one();
    }

    bool pop(T& item, std::chrono::milliseconds timeout)
    {
        std::unique_lock<std::mutex> lock(m_lock);
        if (!m_signal.wait_for(lock, timeout, [&] {
            return m_queue.size() || m_done;
        }))
            return false;
        if (m_done)
            return false;
        item = m_queue.front();
        m_queue.pop();
        return true;
    }

    void close()
    {
        std::unique_lock<std::mutex> lock(m_lock);
        m_done = true;
        m_signal.notify_one();
    }

private:
    bool m_done; // FIXME: data member initializer not supported yet
    std::queue<T> m_queue;
    std::mutex m_lock;
    std::condition_variable m_signal;
};


