#pragma once

#include <thread>

/// A wrapper for std::thread which can't be detached, and which will always
/// join in its destructor (if the thread is joinable).
class scoped_thread final {
public:
    explicit scoped_thread(std::thread&& t);
    ~scoped_thread() noexcept;

private:
    std::thread t_;
};

void swap(scoped_thread& x, scoped_thread& y) noexcept;
