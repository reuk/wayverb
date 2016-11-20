#include "utilities/scoped_thread.h"

#include <stdexcept>

namespace util {

scoped_thread::scoped_thread(std::thread t)
        : t_{std::move(t)} {
    if (!t_.joinable()) {
        throw std::logic_error("No thread.");
    }
}

scoped_thread::~scoped_thread() noexcept {
    t_.join();
}

}  // namespace util
