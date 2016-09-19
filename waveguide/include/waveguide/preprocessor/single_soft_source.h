#pragma once

#include "common/cl/common.h"

namespace waveguide {
namespace preprocessor {

template <typename It>
class single_soft_source final {
public:
    single_soft_source(size_t node, It begin, It end)
            : node_{node}
            , begin_{begin}
            , end_{end} {}

    void operator()(cl::CommandQueue& queue, cl::Buffer& buffer, size_t) {
        const cl_float input_pressure = begin_ != end_ ? *begin_++ : 0;
        const auto current_pressure{
                read_single_value<cl_float>(queue, buffer, node_)};
        const auto new_pressure{current_pressure + input_pressure};
        write_single_value(queue, buffer, node_, new_pressure);
    }

private:
    size_t node_;
    It begin_;
    It end_;
};

template <typename It>
auto make_single_soft_source(size_t node, It begin, It end) {
    return single_soft_source<It>{node, begin, end};
}

}  // namespace preprocessor
}  // namespace waveguide
