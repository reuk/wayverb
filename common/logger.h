#pragma once

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <mutex>

#include "string_builder.h"

class Logger {
public:
    Logger() = delete;

    static void restart();

    template <typename... Ts>
    static void log(Ts &&... ts) {
        std::lock_guard<std::mutex> lock(mutex);
        auto str = get_string(ts...);
        std::ofstream of(fname, std::ofstream::app);
        of << str << std::flush;
    }

    template <typename... Ts>
    static void log_err(Ts &&... ts) {
        std::lock_guard<std::mutex> lock(mutex);
        auto str = get_string(ts...);
        std::ofstream of(fname, std::ofstream::app);
        of << str << std::flush;
        std::cerr << str << std::flush;
    }

    class ScopedLog {
    public:
        ScopedLog(const std::string &t)
                : t(t) {
            Logger::log(t, ": begin");
        }
        virtual ~ScopedLog() noexcept {
            Logger::log(t, ": end");
        }

    private:
        std::string t;
    };

private:
    static std::mutex mutex;
    static const std::string fname;

    template <typename... Ts>
    static std::string get_string(Ts &&... ts) {
        std::stringstream ss;
#ifdef __APPLE__
        auto t0 = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now());
        auto t1 = localtime(&t0);
        ss << std::put_time(t1, "%a %b %d %H:%M:%S %Y") << ": ";
#endif
        ss << build_string(ts...) << std::endl;
        return ss.str();
    }
};

#define LOG_SCOPE Logger::ScopedLog sl_##__COUNTER__(__FUNCTION__);
