#pragma once

#include "common/cl/common.h"

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
                read_value<cl_float>(queue, buffer, node_);
        write_value(queue, buffer, node_, current_pressure + *begin_++);
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
