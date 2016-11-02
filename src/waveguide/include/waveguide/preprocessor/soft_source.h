#pragma once

#include "core/cl/common.h"

namespace wayverb {
namespace waveguide {
namespace preprocessor {

template <typename It>
class soft_source final {
public:
    soft_source(size_t node, It begin, It end)
            : node_{node}
            , begin_{begin}
            , end_{end} {}

    bool operator()(cl::CommandQueue& queue, cl::Buffer& buffer, size_t) {
        if (begin_ == end_) {
            return false;
        }
        const auto current_pressure =
                core::read_value<cl_float>(queue, buffer, node_);
        core::write_value(queue, buffer, node_, current_pressure + *begin_++);
        return true;
    }

private:
    size_t node_;
    It begin_;
    It end_;
};

template <typename It>
auto make_soft_source(size_t node, It begin, It end) {
    return soft_source<It>{node, begin, end};
}

}  // namespace preprocessor
}  // namespace waveguide
}  // namespace wayverb
