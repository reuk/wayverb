#pragma once

#include <thread>

namespace util {

/// A wrapper for std::thread which can't be detached, and which will always
/// join in its destructor (if the thread is joinable).
class scoped_thread final {
public:
    scoped_thread() = default;
    explicit scoped_thread(std::thread t);
    ~scoped_thread() noexcept;

    scoped_thread(scoped_thread&&) noexcept = default;
    scoped_thread& operator=(scoped_thread&&) noexcept = default;

private:
    std::thread t_;
};

}  // namespace util
