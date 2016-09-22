#pragma once

#include "common/cl/common.h"

namespace waveguide {
namespace preprocessor {

template <typename It>
class hard_source final : public base {
public:
    hard_source(size_t node, It begin, It end)
            : node_{node}
            , begin_{begin}
            , end_{end}
            , steps_{std::distance(begin, end)} {}

    bool operator()(cl::CommandQueue& queue, cl::Buffer& buffer, size_t) {
        const cl_float new_pressure = begin_ != end_ ? *begin_++ : 0;
        write_value(queue, buffer, node_, new_pressure);
        return begin_ != end_;
    }

private:
    size_t node_;
    It begin_;
    It end_;
    decltype(std::distance(std::declval<It>(), std::declval<It>())) steps_;
};

template <typename It>
auto make_hard_source(size_t node, It begin, It end) {
    return hard_source<It>{node, begin, end};
}

}  // namespace preprocessor
}  // namespace waveguide
