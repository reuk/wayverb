#include "timed_scope.h"
#include "logger.h"

TimedScope::TimedScope(const std::string& name)
        : name(name) {
}

TimedScope::~TimedScope() noexcept {
    auto tock = std::chrono::steady_clock::now();
    Logger::log_err(
        "scope \"",
        name,
        "\" completed in: ",
        std::chrono::duration_cast<std::chrono::milliseconds>(tock - tick)
            .count(),
        " ms");
}
