#include "waveguide/mesh_descriptor.h"
#include "waveguide/postprocessor/directional_receiver.h"

#include "common/cl/common.h"
#include "common/map_to_vector.h"

namespace waveguide {
namespace postprocessor {

directional_receiver::directional_receiver(
        const mesh_descriptor& mesh_descriptor,
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
                    "can't place directional_receiver at this node as it is "
                    "adjacent to "
                    "a boundary");
        }
    }
}

directional_receiver::return_type directional_receiver::operator()(
        cl::CommandQueue& queue, const cl::Buffer& buffer, size_t) {
    //  copy out node pressure
    const auto pressure{read_value<cl_float>(queue, buffer, output_node_)};

    //  copy out surrounding pressures
    auto surrounding{map_to_vector(surrounding_nodes_, [&](auto i) {
        return read_value<cl_float>(queue, buffer, i);
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

    return {intensity, pressure};
}

size_t directional_receiver::get_output_node() const { return output_node_; }

}  // namespace postprocessor
}  // namespace waveguide
