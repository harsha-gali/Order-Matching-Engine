#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

/**
 * @brief A thread-safe queue with blocking and non-blocking pop support.
 * 
 * This queue uses a mutex and condition_variable to ensure safe concurrent
 * access from multiple producer and consumer threads. It supports both
 * copy and move semantics for pushing, and exposes blocking and try-pop methods.
 */
template<typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() = default;
    ~ThreadSafeQueue() = default;

    // Non-copyable
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

    // Add item (copy)
    void push(const T& item) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(item);
        cond_var_.notify_one();
    }

    // Add item (move)
    void push(T&& item) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(item));
        cond_var_.notify_one();
    }

    /**
     * @brief Blocks until the queue is non-empty, then pops front into `out`.
     */
    void wait_and_pop(T& out) {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_var_.wait(lock, [this] { return !queue_.empty(); });
        out = std::move(queue_.front());
        queue_.pop();
    }

    /**
     * @brief Non-blocking pop. Returns std::nullopt if empty.
     */
    std::optional<T> try_pop() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) return std::nullopt;
        T item = std::move(queue_.front());
        queue_.pop();
        return item;
    }

    /**
     * @brief Check whether the queue is empty.
     */
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    /**
     * @brief Get current queue size. May be inaccurate in multi-threaded contexts.
     */
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

private:
    mutable std::mutex mutex_;
    std::queue<T> queue_;
    std::condition_variable cond_var_;
};
