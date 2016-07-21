#pragma once

#include "config.h"
#include "rectangular_mesh.h"
#include "rectangular_program.h"

#include "glm/glm.hpp"

#include <algorithm>
#include <array>
#include <type_traits>

class rectangular_waveguide final {
public:
    rectangular_waveguide(const cl::Context&,
                          const cl::Device&,
                          const MeshBoundary& boundary,
                          const glm::vec3& anchor,
                          double sr);
    rectangular_waveguide(const rectangular_waveguide&) = delete;
    rectangular_waveguide& operator=(const rectangular_waveguide&) = delete;
    rectangular_waveguide(rectangular_waveguide&&) noexcept        = delete;
    rectangular_waveguide& operator=(rectangular_waveguide&&) noexcept = delete;
    ~rectangular_waveguide() noexcept;

    const rectangular_mesh& get_mesh() const;

    size_t get_index_for_coordinate(const glm::vec3& v) const;
    glm::vec3 get_coordinate_for_index(size_t index) const;
    bool inside(size_t index) const;

    class run_step_output final {
    public:
        explicit run_step_output(float pressure             = 0,
                                 const glm::vec3& intensity = glm::vec3())
                : pressure(pressure)
                , intensity(intensity) {}

        float get_pressure() const { return pressure; }
        glm::vec3 get_intensity() const { return intensity; }

    private:
        float pressure;
        glm::vec3 intensity;
    };

    using per_step_callback   = std::function<void()>;
    using visualiser_callback = std::function<void(aligned::vector<float>)>;

    aligned::vector<run_step_output> init_and_run(
            const glm::vec3& e,
            const aligned::vector<float>& input,
            size_t o,
            size_t steps,
            std::atomic_bool& keep_going,
            const per_step_callback& callback);

    aligned::vector<run_step_output> init_and_run_visualised(
            const glm::vec3& e,
            const aligned::vector<float>& input,
            size_t o,
            size_t steps,
            std::atomic_bool& keep_going,
            const per_step_callback& callback,
            const visualiser_callback& visual_callback);

private:
    using kernel_type =
            decltype(std::declval<rectangular_program>().get_kernel());
    static constexpr auto num_ports{rectangular_mesh::num_ports};

    class run_info final {
    public:
        run_info(size_t input_index,
                 const aligned::vector<float>& signal,
                 size_t output_index)
                : input_index(input_index)
                , signal(signal)
                , output_index(output_index) {}

        size_t get_input_index() const { return input_index; }
        const aligned::vector<float>& get_signal() const { return signal; }
        size_t get_output_index() const { return output_index; }

    private:
        size_t input_index;
        aligned::vector<float> signal;
        size_t output_index;
    };

    class run_step_input final {
    public:
        explicit run_step_input(size_t index = 0, float pressure = 0)
                : index(index)
                , pressure(pressure) {}

        size_t get_index() const { return index; }
        float get_pressure() const { return pressure; }

    private:
        size_t index;
        float pressure;
    };

    aligned::vector<run_step_output> run(const run_info& ri,
                                         std::atomic_bool& keep_going,
                                         const per_step_callback& callback);

    aligned::vector<run_step_output> run_visualised(
            const run_info& ri,
            std::atomic_bool& keep_going,
            const per_step_callback& callback,
            const visualiser_callback& visual_callback);

    using input_callback =
            std::function<run_step_output(const run_info&, float)>;

    aligned::vector<run_step_output> run_basic(const run_info& run_info,
                                               std::atomic_bool& keep_going,
                                               const input_callback& callback);

    run_info init(const glm::vec3& e,
                  const aligned::vector<float>& input_sig,
                  size_t o,
                  size_t steps);

    run_step_output run_step(const run_info& run_info, float input);

    std::pair<run_step_output, aligned::vector<cl_float>> run_step_visualised(
            const run_info& run_info, float input);

    run_step_output run_step(const run_step_input& run_step_input,
                             size_t o,
                             cl::CommandQueue& queue,
                             kernel_type& kernel,
                             size_t nodes,
                             cl::Buffer& previous,
                             cl::Buffer& current,
                             cl::Buffer& output);

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

    template <typename T>
    void write_single_value(cl::Buffer& buffer, size_t index, T val) {
        queue.enqueueWriteBuffer(
                buffer, CL_TRUE, sizeof(T) * index, sizeof(T), &val);
    }

    template <typename T>
    T read_single_value(const cl::Buffer& buffer, size_t index) {
        T ret;
        queue.enqueueReadBuffer(
                buffer, CL_TRUE, sizeof(T) * index, sizeof(T), &ret);
        return ret;
    }

    struct rectangular_waveguide_run_info;
    std::unique_ptr<rectangular_waveguide_run_info> invocation;

    cl::CommandQueue queue;
    const rectangular_program program;
    kernel_type kernel;

    const rectangular_mesh mesh;
    const size_t nodes;

    cl::Buffer previous;
    cl::Buffer current;

    cl::Buffer output;

    const double sample_rate;

    const cl::Buffer node_buffer;
    const cl::Buffer boundary_coefficients_buffer;
    cl::Buffer error_flag_buffer;

    friend bool operator==(const rectangular_waveguide& a,
                           const rectangular_waveguide& b);
};

bool operator==(const rectangular_waveguide& a, const rectangular_waveguide& b);
