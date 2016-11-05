#pragma once

#include "utilities/threading_policies.h"

#include <queue>

namespace util {

template <typename ThreadingPolicy, typename... Ts>
class work_queue final {
public:
    template <typename Method>
    void push(Method&& method) {
        const auto lck = threading_policy_.get_lock();
        work_items.push(std::forward<Method>(method));
    }

    std::function<void(Ts...)> pop() {
        const auto lck = threading_policy_.get_lock();
        if (work_items.empty()) {
            return std::function<void(Ts...)>{};
        }
        auto ret = std::move(work_items.front());
        work_items.pop();
        return ret;
    }

    size_t size() const {
        const auto lck = threading_policy_.get_lock();
        return work_items.size();
    }

    bool empty() const {
        const auto lck = threading_policy_.get_lock();
        return work_items.empty();
    }

    void clear() const {
        const auto lck = threading_policy_.get_lock();
        work_items.clear();
    }

private:
    ThreadingPolicy threading_policy_;    
    std::queue<std::function<void(Ts...)>> work_items;
};

}  // namespace util
