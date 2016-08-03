#pragma once

#include "waveguide/waveguide.h"

namespace waveguide {
namespace preprocessor {

class single_soft_source final {
public:
    single_soft_source(size_t node, const aligned::vector<float>& signal);
    void operator()(cl::CommandQueue& queue, cl::Buffer& buffer, size_t step);

private:
    size_t node;
    aligned::vector<float> signal;
};

}  // namespace preprocessor
}  // namespace waveguide
