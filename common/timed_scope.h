#pragma once

#include <chrono>
#include <string>

class TimedScope {
public:
    TimedScope(const std::string& name = "");
    virtual ~TimedScope() noexcept;

    TimedScope(const TimedScope&) = delete;
    TimedScope& operator=(const TimedScope&) = delete;
    TimedScope(TimedScope&&) noexcept = delete;
    TimedScope& operator=(TimedScope&&) noexcept = delete;

private:
    std::string name;
    std::chrono::time_point<std::chrono::steady_clock> tick{
        std::chrono::steady_clock::now()};
};
