#pragma once

#include <thread>

class single_thread_access_checker final {
public:
    single_thread_access_checker();
    void operator()() const;

private:
    std::thread::id id_;
};
