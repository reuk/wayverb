#pragma once

#include <mutex>
#include <queue>

class WorkQueue {
public:
    struct WorkItem {
        virtual ~WorkItem() noexcept = default;
        virtual void operator()() = 0;
    };

    template <typename Method>
    void push(Method&& method) {
        std::lock_guard<std::mutex> lck(mut);
        work_items.push(std::make_unique<GenericWorkItem<Method>>(
                std::forward<Method>(method)));
    }

    std::unique_ptr<WorkItem> pop();

    size_t size() const;
    bool empty() const;

private:
    template <typename Method>
    struct GenericWorkItem : public WorkItem {
        GenericWorkItem(Method&& method)
                : method(std::forward<Method>(method)) {
        }
        void operator()() override {
            method();
        }
        Method method;
    };

    std::queue<std::unique_ptr<WorkItem>> work_items;
    mutable std::mutex mut;
};