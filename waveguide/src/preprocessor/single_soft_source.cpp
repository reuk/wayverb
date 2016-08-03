#include "waveguide/preprocessor/single_soft_source.h"

namespace waveguide {
namespace preprocessor {

single_soft_source::single_soft_source(size_t node,
                                       const aligned::vector<float>& signal)
        : node(node)
        , signal(signal) {}

void single_soft_source::operator()(cl::CommandQueue& queue,
                                    cl::Buffer& buffer,
                                    size_t step) {
    const auto input_pressure = signal.size() <= step ? 0 : signal[step];
    const auto current_pressure =
            read_single_value<cl_float>(queue, buffer, node);
    const cl_float new_pressure = current_pressure + input_pressure;
    write_single_value(queue, buffer, node, new_pressure);
}

}  // namespace preprocessor
}  // namespace waveguide
