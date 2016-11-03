#include "scoped_thread.hpp"

scoped_thread::scoped_thread(std::thread&& t)
        : t_(std::move(t)) {
    if (!t_.joinable()) {
        throw std::logic_error("scoped_thread: no thread");
    }
}

scoped_thread::~scoped_thread() noexcept {
    t_.join();
}
