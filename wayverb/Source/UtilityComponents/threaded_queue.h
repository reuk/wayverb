#pragma once

#include <experimental/optional>
#include <mutex>
#include <queue>

template <typename T>
class threaded_queue final {
public:
    using value_type = T;
    
    void push(value_type value) {
        std::lock_guard<std::mutex> lck{mutex_};
        queue_.push(std::move(value));
    }

    std::experimental::optional<value_type> pop() {
        std::lock_guard<std::mutex> lck{mutex_};
        if (queue_.empty()) {
            return std::experimental::nullopt;
        }
        auto ret = std::move(queue_.front());
        queue_.pop();
        return ret;
    }

    void clear() {
        std::lock_guard<std::mutex> lck{mutex_};
        queue_ = std::queue<value_type>{};
    }

private:
    mutable std::mutex mutex_;
    std::queue<value_type> queue_;
};
