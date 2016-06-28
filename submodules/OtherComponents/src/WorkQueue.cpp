#include "OtherComponents/WorkQueue.hpp"

std::unique_ptr<WorkQueue::WorkItem> WorkQueue::pop() {
    std::lock_guard<std::mutex> lck(mut);
    if (work_items.empty()) {
        return nullptr;
    }
    auto ret = std::move(work_items.front());
    work_items.pop();
    return ret;
}

size_t WorkQueue::size() const {
    std::lock_guard<std::mutex> lck(mut);
    return work_items.size();
}

bool WorkQueue::empty() const {
    std::lock_guard<std::mutex> lck(mut);
    return work_items.empty();
}