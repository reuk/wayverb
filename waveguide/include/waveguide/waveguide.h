#pragma once

#include "config.h"
#include "mesh.h"
#include "program.h"

#include "glm/glm.hpp"

#include <algorithm>
#include <array>
#include <experimental/optional>
#include <type_traits>

namespace waveguide {

class waveguide final {
public:
    using per_step_callback = std::function<void()>;
    using visualiser_callback =
            std::function<void(const aligned::vector<cl_float>&)>;

    waveguide(const cl::Context&,
              const cl::Device&,
              const MeshBoundary& boundary,
              const glm::vec3& anchor,
              double sample_rate);
    waveguide(const waveguide&) = delete;
    waveguide& operator=(const waveguide&) = delete;
    waveguide(waveguide&&) noexcept        = delete;
    waveguide& operator=(waveguide&&) noexcept = delete;
    ~waveguide() noexcept;

    const mesh& get_mesh() const;
    double get_sample_rate() const;

    size_t get_index_for_coordinate(const glm::vec3& v) const;
    glm::vec3 get_coordinate_for_index(size_t index) const;
    bool inside(size_t index) const;

    using step_postprocessor =
            std::function<void(cl::CommandQueue&, const cl::Buffer&)>;

    //  returns whether the run was successful
    bool init_and_run(const glm::vec3& excitation_location,
                      aligned::vector<float> input,
                      size_t steps,
                      const aligned::vector<step_postprocessor>& postprocessors,
                      const per_step_callback& callback,
                      std::atomic_bool& keep_going);

private:
    using kernel_type = decltype(std::declval<program>().get_kernel());

    waveguide(const cl::Context&,
              const cl::Device&,
              const mesh& mesh,
              double sample_rate,
              aligned::vector<program::CanonicalCoefficients> coefficients);
    waveguide(const cl::Context&,
              const cl::Device&,
              const mesh& mesh,
              double sample_rate,
              aligned::vector<program::CondensedNodeStruct> nodes,
              aligned::vector<program::CanonicalCoefficients> coefficients);

    cl::CommandQueue queue;
    const program program;
    kernel_type kernel;

    const mesh lattice;

    double sample_rate;

    cl::Buffer previous;
    cl::Buffer current;

    const cl::Buffer node_buffer;
    const cl::Buffer boundary_coefficients_buffer;
    cl::Buffer error_flag_buffer;
};

//----------------------------------------------------------------------------//

struct run_step_output {
    glm::vec3 intensity;
    float pressure;
};

std::experimental::optional<aligned::vector<run_step_output>> init_and_run(
        waveguide& waveguide,
        const glm::vec3& e,
        const aligned::vector<float>& input,
        size_t output_node,
        size_t steps,
        std::atomic_bool& keep_going,
        const waveguide::per_step_callback& callback);

std::experimental::optional<aligned::vector<run_step_output>> init_and_run(
        waveguide& waveguide,
        const glm::vec3& e,
        const aligned::vector<float>& input,
        size_t output_node,
        size_t steps,
        std::atomic_bool& keep_going,
        const waveguide::per_step_callback& callback,
        const waveguide::visualiser_callback& visual_callback);

}  // namespace waveguide
