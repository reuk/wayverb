#pragma once

#include "utilities/threaded_queue.h"

#include "../JuceLibraryCode/JuceHeader.h"

/// Assuming the work_queue is thread-safe (and I'm pretty confident it is)
/// we don't need any locks here.
/// work_queue.push is thread-safe
/// triggerAsyncUpdate is thread-safe
/// handleAsyncUpdate will only be called on the message thread, and calls to
/// work_queue_.pop will block if something is being pushed from another thread.
class async_work_queue final : private juce::AsyncUpdater {
public:
    using queue = util::threaded_queue<util::threading_policy::scoped_lock,
                                       std::function<void()>>;

    void push(queue::value_type method);

private:
    void handleAsyncUpdate() override;

    queue work_queue_;
};
