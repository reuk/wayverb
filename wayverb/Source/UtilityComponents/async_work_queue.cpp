#include "async_work_queue.h"

/// Assuming the work_queue is thread-safe (and I'm pretty confident it is)
/// we don't need any locks here.
/// work_queue.push is thread-safe
/// triggerAsyncUpdate is thread-safe
/// handleAsyncUpdate will only be called on the message thread, and calls to
/// work_queue_.pop will block if something is being pushed from another thread.

void async_work_queue::push(queue::value_type method) {
    work_queue_.push(std::move(method));
    triggerAsyncUpdate();
}

void async_work_queue::handleAsyncUpdate() override {
    while (const auto method = work_queue_.pop()) {
        (*method)();
    }
}
