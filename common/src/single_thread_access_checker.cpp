#include "common/single_thread_access_checker.h"

#include <cassert>

single_thread_access_checker::single_thread_access_checker()
        : id_(std::this_thread::get_id()) {}

bool single_thread_access_checker::is_same_thread() const {
    return id_ == std::this_thread::get_id();
}