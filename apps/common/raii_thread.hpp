#pragma once

#include <thread>

/// A wrapper for std::thread which can't be detached, and which will always
/// join in its destructor (if the thread is joinable).
class raii_thread final {
public:
    raii_thread() noexcept = default;

    template <class Fn, class... Args>
    explicit raii_thread(Fn&& fn, Args&&... args)
            : thread(std::forward<Fn>(fn), std::forward<Args>(args)...) {
    }

    raii_thread(const raii_thread&) = delete;
    raii_thread(raii_thread&&) noexcept = default;

    ~raii_thread() noexcept;

    raii_thread& operator=(const raii_thread&) = delete;
    raii_thread& operator=(raii_thread&&) noexcept = default;

    std::thread::id get_id() const noexcept;

    bool joinable() const noexcept;
    void join();

    void swap(raii_thread& x) noexcept;

private:
    std::thread thread;
};

void swap(raii_thread& x, raii_thread& y) noexcept;