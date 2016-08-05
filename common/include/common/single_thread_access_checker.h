#pragma once

#include "common/access_wrapper.h"

#include <thread>
#include <cassert>

class single_thread_access_checker final {
public:
    single_thread_access_checker();
    bool is_same_thread() const;

private:
    std::thread::id id_;
};

template <typename T>
class single_thread_access_wrapper final : public access_wrapper<T> {
public:
    using access_wrapper<T>::access_wrapper;
    using access_wrapper<T>::operator=;
private:
    void callback() override {
        assert(single_thread_access_checker_.is_same_thread());
    }

    single_thread_access_checker single_thread_access_checker_;
};