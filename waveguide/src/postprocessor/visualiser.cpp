#include "waveguide/postprocessor/visualiser.h"

namespace waveguide {
namespace postprocessor {

visualiser::visualiser(output_callback callback)
        : callback_(std::move(callback)) {}

void visualiser::operator()(cl::CommandQueue& queue,
                            const cl::Buffer& buffer,
                            size_t step) {
    callback_(read_from_buffer<cl_float>(queue, buffer), step);
}

}  // namespace postprocessor
}  // namespace waveguide
