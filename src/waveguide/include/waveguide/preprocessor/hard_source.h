#pragma once

#include "core/cl/common.h"

namespace waveguide {
namespace preprocessor {

template <typename It>
class hard_source final {
public:
    hard_source(size_t node, It begin, It end)
            : node_{node}
            , begin_{begin}
            , end_{end} {}

    bool operator()(cl::CommandQueue& queue, cl::Buffer& buffer, size_t) {
        if (begin_ == end_) {
            return false;
        }
        write_value(queue, buffer, node_, *begin_++);
        return true;
    }

private:
    size_t node_;
    It begin_;
    It end_;
};

template <typename It>
auto make_hard_source(size_t node, It begin, It end) {
    return hard_source<It>{node, begin, end};
}

}  // namespace preprocessor
}  // namespace waveguide
