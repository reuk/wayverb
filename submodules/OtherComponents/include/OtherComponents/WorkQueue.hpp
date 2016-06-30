#pragma once

#include <mutex>
#include <queue>

template <typename... Ts>
class WorkQueue {
public:
    template <typename Method>
    void push(Method&& method) {
        std::lock_guard<std::mutex> lck(mut);
        work_items.push(std::make_unique<std::function<void(Ts...)>>(
                std::forward<Method>(method)));
    }

    std::unique_ptr<std::function<void(Ts...)>> pop() {
        std::lock_guard<std::mutex> lck(mut);
        if (work_items.empty()) {
            return nullptr;
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
    std::queue<std::unique_ptr<std::function<void(Ts...)>>> work_items;
};