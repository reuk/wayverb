#include "common/timed_scope.h"

#include <glog/logging.h>

TimedScope::TimedScope(const std::string& name)
        : name(name) {
}

TimedScope::~TimedScope() noexcept {
    auto tock = std::chrono::steady_clock::now();
    LOG(INFO) << "scope \"" << name << "\" completed in: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(tock -
                                                                       tick)
                         .count()
              << " ms";
}
