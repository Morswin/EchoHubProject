#ifndef ECHOHUB_THREAD_SAFE_QUEUE_HPP
#define ECHOHUB_THREAD_SAFE_QUEUE_HPP

#include <queue>
#include <mutex>
#include <condition_variable>
#include <utility> // for std::move

/**
 * @brief Thread-safe queue for producer-consumer pattern.
 * 
 * Allows safe data exchange between threads. Producer threads can push data,
 * and consumer threads can pop data without race conditions.
 * 
 * @tparam T Type of elements stored in the queue.
 */
template <typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cond_var_;

public:
    ThreadSafeQueue() = default;

    // Copying is not allowed (mutexes cannot be copied)
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

    /**
     * @brief Push an item to the queue (used by producer thread).
     * @param item The item to push.
     */
    void push(T item) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(item));
        cond_var_.notify_one(); // Wake up one waiting thread
    }

    /**
     * @brief Pop an item from the queue (used by consumer thread).
     * @return The popped item.
     * 
     * If the queue is empty, the thread will sleep until data is available.
     */
    T pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_var_.wait(lock, [this]() { return !queue_.empty(); });
        T item = std::move(queue_.front());
        queue_.pop();
        return item;
    }

    /**
     * @brief Try to pop an item from the queue without waiting.
     * @param item Output parameter for the popped item.
     * @return true if an item was popped, false if the queue was empty.
     */
    bool tryPop(T& item) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        }
        item = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    /**
     * @brief Check if the queue is empty.
     * @return true if the queue is empty, false otherwise.
     */
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    /**
     * @brief Get the number of elements in the queue.
     * @return Size of the queue.
     */
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    /**
     * @brief Clear all elements from the queue.
     */
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        while (!queue_.empty()) {
            queue_.pop();
        }
    }
};

#endif // ECHOHUB_THREAD_SAFE_QUEUE_HPP
