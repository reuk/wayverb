#pragma once

#include <mutex>

namespace util {
namespace threading_policy {

class no_lock final {
public:
    auto get_lock() const {
        struct ret {};
        return ret{};
    }
};

class scoped_lock final {
public:
    auto get_lock() const { return std::unique_lock<std::mutex>{mutex_}; }

private:
    mutable std::mutex mutex_;
};

}  // namespace threading_policy
}  // namespace util
