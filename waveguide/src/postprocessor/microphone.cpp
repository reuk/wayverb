#include "waveguide/postprocessor/microphone.h"

namespace waveguide {
namespace postprocessor {

microphone::microphone(const mesh& mesh,
                       size_t output_node,
                       double sample_rate,
                       const output_callback& callback)
        : output_node(output_node)
        , surrounding_nodes(mesh.compute_neighbors(output_node))
        , mesh_spacing(mesh.get_spacing())
        , sample_rate(sample_rate)
        , callback(callback) {}

void microphone::process(cl::CommandQueue& queue, const cl::Buffer& buffer) {
    //  copy out node pressure
    const auto pressure =
            read_single_value<cl_float>(queue, buffer, output_node);

    //  copy out surrounding pressures
    aligned::vector<cl_float> surrounding;
    surrounding.reserve(surrounding_nodes.size());
    for (auto i : surrounding_nodes) {
        surrounding.push_back(read_single_value<cl_float>(queue, buffer, i));
    }

    //  pressure difference vector is obtained by subtracting the central
    //  junction pressure from the pressure values of neighboring junctions
    for (auto& i : surrounding) {
        i -= pressure;
        //  and dividing these terms by the spatial sampling period
        i /= mesh_spacing;
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
    static constexpr auto ambient_density = 1.225;
    const auto dv                         = m / -ambient_density;
    //  and integrated using a discrete-time integrator
    velocity += (1.0 / sample_rate) * dv;

    //  the instantaneous intensity is obtained by multiplying the velocity
    //  and the pressure
    const auto intensity = velocity * static_cast<double>(pressure);

    callback(pressure, intensity);
}

}  // namespace postprocessor
}  // namespace waveguide
