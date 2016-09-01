#include "waveguide/postprocessor/microphone.h"
#include "waveguide/descriptor.h"

#include "common/map_to_vector.h"

namespace waveguide {
namespace postprocessor {
namespace detail {
microphone_state::microphone_state(const descriptor& mesh_descriptor,
                                   double sample_rate,
                                   double ambient_density,
                                   size_t output_node)
        : mesh_spacing_(mesh_descriptor.spacing)
        , sample_rate_(sample_rate)
        , ambient_density_(ambient_density)
        , output_node_(output_node)
        , surrounding_nodes_(compute_neighbors(mesh_descriptor, output_node)) {
    for (auto i : surrounding_nodes_) {
        if (i == ~cl_uint{0}) {
            throw std::runtime_error(
                    "can't place microphone at this node as it is adjacent to "
                    "a boundary");
        }
    }
}

run_step_output microphone_state::operator()(cl::CommandQueue& queue,
                                             const cl::Buffer& buffer,
                                             size_t) {
    //  copy out node pressure
    const auto pressure{
            read_single_value<cl_float>(queue, buffer, output_node_)};

    //  copy out surrounding pressures
    auto surrounding{map_to_vector(surrounding_nodes_, [&](auto i) {
        return read_single_value<cl_float>(queue, buffer, i);
    })};

    //  pressure difference vector is obtained by subtracting the central
    //  junction pressure from the pressure values of neighboring junctions
    for (auto& i : surrounding) {
        i -= pressure;
        //  and dividing these terms by the spatial sampling period
        i /= mesh_spacing_;
    }

    //  the approximation of the pressure gradient is obtained by
    //  multiplying the difference vector by the inverse projection matrix,
    //  which looks like
    //      -0.5  0.5    0    0    0    0
    //         0    0 -0.5  0.5    0    0
    //         0    0    0    0 -0.5  0.5
    //  so I think the product is like this:
    const glm::dvec3 m{surrounding[0] * -0.5 + surrounding[1] * 0.5,
                       surrounding[2] * -0.5 + surrounding[3] * 0.5,
                       surrounding[4] * -0.5 + surrounding[5] * 0.5};

    //  the result is scaled by the negative inverse of the ambient density
    const auto dv{m / -ambient_density_};
    //  and integrated using a discrete-time integrator
    velocity_ += (1.0 / sample_rate_) * dv;

    //  the instantaneous intensity is obtained by multiplying the velocity
    //  and the pressure
    const auto intensity{velocity_ * static_cast<double>(pressure)};

    return run_step_output{intensity, pressure};
}

size_t microphone_state::get_output_node() const { return output_node_; }
}  // namespace detail

microphone::microphone(const descriptor& mesh_descriptor,
                       double sample_rate,
                       double ambient_density,
                       size_t output_node,
                       output_callback callback)
        : microphone_state_(
                  mesh_descriptor, sample_rate, ambient_density, output_node)
        , callback_(std::move(callback)) {
    if (ambient_density < 0.5 || 10 < ambient_density) {
        throw std::runtime_error{
                "ambient density value looks a bit suspicious"};
    }
}

void microphone::operator()(cl::CommandQueue& queue,
                            const cl::Buffer& buffer,
                            size_t step) {
    callback_(microphone_state_(queue, buffer, step));
}

}  // namespace postprocessor
}  // namespace waveguide
