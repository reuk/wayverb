#include "raii_thread.hpp"

raii_thread::~raii_thread() noexcept {
    if (thread.joinable()) {
        thread.join();
    }
}

std::thread::id raii_thread::get_id() const noexcept {
    return thread.get_id();
}

bool raii_thread::joinable() const noexcept {
    return thread.joinable();
}

void raii_thread::join() {
    thread.join();
}

void raii_thread::swap(raii_thread& x) noexcept {
    thread.swap(x.thread);
}

void swap(raii_thread& x, raii_thread& y) noexcept {
    x.swap(y);
}