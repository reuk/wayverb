#pragma once

#include "utilities/work_queue.h"

#include "../JuceLibraryCode/JuceHeader.h"

template <typename ThreadingPolicy, typename... Ts>
class async_work_queue final : private juce::AsyncUpdater {
public:
    template <typename Method>
    void push(Method&& method) {
        const auto lck = threading_policy_.get_lock();
        work_queue_.push(std::forward<Method>(method));
        triggerAsyncUpdate();
    }

private:
    inline void handleAsyncUpdate() override {
        const auto lck = threading_policy_.get_lock();
        while (const auto method = work_queue_.pop()) {
            method();
        }
    }

    ThreadingPolicy threading_policy_;
    util::work_queue<util::threading_policy::no_lock, Ts...> work_queue_;
};
