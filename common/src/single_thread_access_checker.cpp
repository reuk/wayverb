#include "common/single_thread_access_checker.h"

#include <cassert>

single_thread_access_checker::single_thread_access_checker()
        : id_(std::this_thread::get_id()) {}
void single_thread_access_checker::operator()() const {
    assert(id_ == std::this_thread::get_id());
}
