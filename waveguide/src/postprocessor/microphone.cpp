#include "waveguide/postprocessor/microphone.h"
#include "waveguide/descriptor.h"

#include "common/map_to_vector.h"

namespace waveguide {
namespace postprocessor {

namespace detail {
microphone_state::microphone_state(const descriptor& mesh_descriptor,
                                   size_t output_node,
                                   double sample_rate,
                                   double ambient_density)
        : output_node(output_node)
        , surrounding_nodes(compute_neighbors(mesh_descriptor, output_node))
        , mesh_spacing(mesh_descriptor.spacing)
        , sample_rate(sample_rate)
        , ambient_density(ambient_density) {}

run_step_output microphone_state::operator()(cl::CommandQueue& queue,
                                             const cl::Buffer& buffer,
                                             size_t) {
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
    const auto dv = m / -ambient_density;
    //  and integrated using a discrete-time integrator
    velocity += (1.0 / sample_rate) * dv;

    //  the instantaneous intensity is obtained by multiplying the velocity
    //  and the pressure
    const auto intensity = velocity * static_cast<double>(pressure);

    return run_step_output{intensity, pressure};
}

size_t microphone_state::get_output_node() const { return output_node; }
}  // namespace detail

microphone::microphone(const descriptor& mesh_descriptor,
                       size_t output_node,
                       double sample_rate,
                       double ambient_density,
                       const output_callback& callback)
        : microphone_state(
                  mesh_descriptor, output_node, sample_rate, ambient_density)
        , callback(callback) {}

void microphone::operator()(cl::CommandQueue& queue,
                            const cl::Buffer& buffer,
                            size_t step) {
    callback(microphone_state(queue, buffer, step));
}

multi_microphone::multi_microphone(const descriptor& mesh_descriptor,
                                   const aligned::vector<size_t>& output_node,
                                   double sample_rate,
                                   double ambient_density,
                                   const output_callback& callback)
        : state(map_to_vector(
                  output_node,
                  [&](auto i) {
                      return detail::microphone_state(
                              mesh_descriptor, i, sample_rate, ambient_density);
                  }))
        , callback(callback) {}

void multi_microphone::operator()(cl::CommandQueue& queue,
                                  const cl::Buffer& buffer,
                                  size_t step) {
    aligned::vector<std::tuple<run_step_output, size_t>> ret;
    ret.reserve(state.size());
    for (auto& i : state) {
        ret.push_back(
                std::make_tuple(i(queue, buffer, step), i.get_output_node()));
    }
    callback(ret);
}

}  // namespace postprocessor
}  // namespace waveguide
