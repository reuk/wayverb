#pragma once

#include "compensation_signal/compile_time.h"

#include "common/cl/common.h"
#include "common/program_wrapper.h"

#include "utilities/aligned/vector.h"

#include <array>

class compressed_rectangular_waveguide_program final {
public:
    explicit compressed_rectangular_waveguide_program(
            const compute_context& cc);

    auto get_compressed_waveguide_kernel() const {
        return program_wrapper_.get_kernel<cl::Buffer, cl::Buffer>(
                "compressed_waveguide");
    }

    auto get_zero_buffer_kernel() const {
        return program_wrapper_.get_kernel<cl::Buffer>("zero_buffer");
    }

    template <cl_program_info T>
    auto get_info() const {
        return program_wrapper_.get_info<T>();
    }

    cl::Device get_device() const { return program_wrapper_.get_device(); }

private:
    program_wrapper program_wrapper_;
};

////////////////////////////////////////////////////////////////////////////////

class compressed_rectangular_waveguide final {
public:
    using compressed_waveguide_kernel =
            decltype(std::declval<compressed_rectangular_waveguide_program>()
                             .get_compressed_waveguide_kernel());
    using zero_buffer_kernel =
            decltype(std::declval<compressed_rectangular_waveguide_program>()
                             .get_zero_buffer_kernel());

    compressed_rectangular_waveguide(const compute_context& cc, size_t steps);

    template <typename It, typename T>
    aligned::vector<float> run_hard_source(It begin,
                                           It end,
                                           const T& per_step) {
        return run(begin,
                   end,
                   [](auto& queue, auto& buffer, float input) {
                       write_value(queue, buffer, 0, input);
                   },
                   per_step);
    }

    template <typename It, typename T>
    aligned::vector<float> run_soft_source(It begin,
                                           It end,
                                           const T& per_step) {
        return run(begin,
                   end,
                   [](auto& queue, auto& buffer, float input) {
                       const auto c = read_value<cl_float>(queue, buffer, 0);
                       write_value(queue, buffer, 0, c + input);
                   },
                   per_step);
    }

private:
    template <typename It, typename T, typename U>
    aligned::vector<float> run(It begin,
                               It end,
                               const T& writer,
                               const U& per_step) {
        //  init buffers
        const auto buffer_size = tetrahedron(dimension_);
        zero_buffer_kernel_(
                cl::EnqueueArgs{queue_,
                                cl::NDRange{previous_.getInfo<CL_MEM_SIZE>() /
                                            sizeof(cl_float)}},
                previous_);
        zero_buffer_kernel_(
                cl::EnqueueArgs{queue_,
                                cl::NDRange{current_.getInfo<CL_MEM_SIZE>() /
                                            sizeof(cl_float)}},
                current_);

        aligned::vector<float> ret{};
        ret.reserve(std::distance(begin, end));

        for (auto count = 0ul; count != dimension_ * 2; ++count) {
            writer(queue_, current_, begin == end ? 0.0f : *begin++);

            //  run the kernel
            compressed_waveguide_kernel_(
                    cl::EnqueueArgs{queue_, cl::NDRange{buffer_size}},
                    previous_,
                    current_);

            //  ping-pong the buffers
            using std::swap;
            swap(previous_, current_);

            //  get output value
            ret.emplace_back(read_value<cl_float>(queue_, current_, 0));

            per_step(count);
        }

        return ret;
    }

    cl::CommandQueue queue_;
    compressed_waveguide_kernel compressed_waveguide_kernel_;
    zero_buffer_kernel zero_buffer_kernel_;
    size_t dimension_;

    cl::Buffer current_;
    cl::Buffer previous_;
};
