#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

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

//----------------------------------------------------------------------------//

class AsyncWorkQueue final : private juce::AsyncUpdater {
public:
    template <typename T>
    void push(T&& t) {
        std::lock_guard<std::mutex> lck(mut);
        outgoing_work_queue.push(std::forward<T>(t));
        triggerAsyncUpdate();
    }

private:
    inline void handleAsyncUpdate() override {
        std::lock_guard<std::mutex> lck(mut);
        while (auto method = outgoing_work_queue.pop()) {
            method();
        }
    }

    mutable std::mutex mut;
    WorkQueue<> outgoing_work_queue;
};
