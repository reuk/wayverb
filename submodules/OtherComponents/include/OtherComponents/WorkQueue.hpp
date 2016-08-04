#pragma once

#include <mutex>
#include <queue>

template <typename... Ts>
class WorkQueue {
public:
    template <typename Method>
    void push(Method&& method) {
        std::lock_guard<std::mutex> lck(mut);
        work_items.push(std::forward<Method>(method));
    }

    std::function<void(Ts...)> pop() {
        std::lock_guard<std::mutex> lck(mut);
        if (work_items.empty()) {
            return std::function<void(Ts...)>{};
        }
        auto ret = std::move(work_items.front());
        work_items.pop();
        return ret;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lck(mut);
        return work_items.size();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lck(mut);
        return work_items.empty();
    }

private:
    mutable std::mutex mut;
    std::queue<std::function<void(Ts...)>> work_items;
};