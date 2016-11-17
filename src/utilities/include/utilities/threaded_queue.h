#pragma once

#include "utilities/threading_policies.h"

#include <queue>
#include <experimental/optional>

namespace util {

template <typename ThreadingPolicy, typename T>
class threaded_queue final {
public:
    using value_type = T;
    
    void push(value_type value) {
        const auto lck = threading_policy_.get_lock();
        queue_.push(std::move(value));
    }

    std::experimental::optional<value_type> pop() {
        const auto lck = threading_policy_.get_lock();
        if (queue_.empty()) {
            return std::experimental::nullopt;
        }
        auto ret = std::move(queue_.front());
        queue_.pop();
        return ret;
    }

    void clear() {
        const auto lck = threading_policy_.get_lock();
        queue_ = std::queue<value_type>{};
    }

private:
    ThreadingPolicy threading_policy_;    
    std::queue<value_type> queue_;
};

}  // namespace util
