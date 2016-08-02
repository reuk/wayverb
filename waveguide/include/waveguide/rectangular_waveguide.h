#pragma once

#include "config.h"
#include "rectangular_mesh.h"
#include "rectangular_program.h"

#include "glm/glm.hpp"

#include <algorithm>
#include <array>
#include <experimental/optional>
#include <type_traits>

class rectangular_waveguide final {
public:
    using per_step_callback = std::function<void()>;
    using visualiser_callback =
            std::function<void(const aligned::vector<cl_float>&)>;

    class step_postprocessor {
    public:
        step_postprocessor()                          = default;
        step_postprocessor(const step_postprocessor&) = default;
        step_postprocessor& operator=(const step_postprocessor&) = default;
        step_postprocessor(step_postprocessor&&) noexcept        = default;
        step_postprocessor& operator=(step_postprocessor&&) noexcept = default;
        virtual ~step_postprocessor() noexcept                       = default;

        virtual void process(cl::CommandQueue& queue,
                             const cl::Buffer& buffer) = 0;
    };

    rectangular_waveguide(const cl::Context&,
                          const cl::Device&,
                          const MeshBoundary& boundary,
                          const glm::vec3& anchor,
                          double sample_rate);
    rectangular_waveguide(const rectangular_waveguide&) = delete;
    rectangular_waveguide& operator=(const rectangular_waveguide&) = delete;
    rectangular_waveguide(rectangular_waveguide&&) noexcept        = delete;
    rectangular_waveguide& operator=(rectangular_waveguide&&) noexcept = delete;
    ~rectangular_waveguide() noexcept;

    const rectangular_mesh& get_mesh() const;

    size_t get_index_for_coordinate(const glm::vec3& v) const;
    glm::vec3 get_coordinate_for_index(size_t index) const;
    bool inside(size_t index) const;

    struct run_step_output {
        glm::vec3 intensity;
        float pressure;
    };

    std::experimental::optional<aligned::vector<run_step_output>> init_and_run(
            const glm::vec3& e,
            const aligned::vector<float>& input,
            size_t output_node,
            size_t steps,
            std::atomic_bool& keep_going,
            const per_step_callback& callback);

    std::experimental::optional<aligned::vector<run_step_output>>
    init_and_run_visualised(const glm::vec3& e,
                            const aligned::vector<float>& input,
                            size_t output_node,
                            size_t steps,
                            std::atomic_bool& keep_going,
                            const per_step_callback& callback,
                            const visualiser_callback& visual_callback);

private:
    using kernel_type =
            decltype(std::declval<rectangular_program>().get_kernel());

    rectangular_waveguide(
            const cl::Context&,
            const cl::Device&,
            const rectangular_mesh& mesh,
            double sample_rate,
            aligned::vector<rectangular_program::CanonicalCoefficients>
                    coefficients);
    rectangular_waveguide(
            const cl::Context&,
            const cl::Device&,
            const rectangular_mesh& mesh,
            double sample_rate,
            aligned::vector<rectangular_program::CondensedNodeStruct> nodes,
            aligned::vector<rectangular_program::CanonicalCoefficients>
                    coefficients);

    //  number of steps defined by input length
    //  returns whether the run was successful
    bool init_and_run(
            const glm::vec3& excitation_location,
            const aligned::vector<float>& input,
            const aligned::vector<std::unique_ptr<step_postprocessor>>&
                    postprocessors,
            const per_step_callback& callback,
            std::atomic_bool& keep_going);

    cl::CommandQueue queue;
    const rectangular_program program;
    kernel_type kernel;

    const rectangular_mesh mesh;

    double sample_rate;

    cl::Buffer previous;
    cl::Buffer current;

    const cl::Buffer node_buffer;
    const cl::Buffer boundary_coefficients_buffer;
    cl::Buffer error_flag_buffer;
};
