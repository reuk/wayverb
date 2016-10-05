#pragma once

#include "compensation_signal/compile_time.h"

#include "common/aligned/vector.h"
#include "common/cl/common.h"
#include "common/program_wrapper.h"

#include <array>

class compressed_rectangular_waveguide_program final {
public:
    explicit compressed_rectangular_waveguide_program(
            const compute_context& cc);

    auto get_kernel() const {
        return program_wrapper_.get_kernel<cl::Buffer, cl::Buffer>(
                "compressed_waveguide");
    }

    template <cl_program_info T>
    auto get_info() const {
        return program_wrapper_.get_info<T>();
    }

    cl::Device get_device() const { return program_wrapper_.get_device(); }

private:
    program_wrapper program_wrapper_;
};

//----------------------------------------------------------------------------//

class compressed_rectangular_waveguide final {
public:
    using kernel_type =
            decltype(std::declval<compressed_rectangular_waveguide_program>()
                             .get_kernel());
    compressed_rectangular_waveguide(const compute_context& cc, size_t steps);

    template <typename T>
    aligned::vector<float> run_hard_source(aligned::vector<float> input,
                                           const T& per_step) {
        return run(std::move(input),
                   [](auto& queue, auto& buffer, auto input) {
                       write_value(queue, buffer, 0, input);
                   },
                   per_step);
    }

    template <typename T>
    aligned::vector<float> run_soft_source(aligned::vector<float> input,
                                           const T& per_step) {
        return run(std::move(input),
                   [](auto& queue, auto& buffer, auto input) {
                       const auto c{read_value<cl_float>(queue, buffer, 0)};
                       write_value(queue, buffer, 0, c + input);
                   },
                   per_step);
    }

private:
    template <typename T, typename U>
    aligned::vector<float> run(aligned::vector<float> input,
                               const T& writer,
                               const U& per_step) {
        input.resize(dimension_ * 2);

        //  init buffers
        {
            //  don't want to keep this in scope for too long!
            aligned::vector<cl_float> n(tetrahedron(dimension_ + 1), 0);
            cl::copy(queue_, n.begin(), n.end(), previous_);
            cl::copy(queue_, n.begin(), n.end(), current_);
        }

        aligned::vector<float> ret{};
        ret.reserve(input.size());

        auto count{0ul};
        for (auto it{input.begin()}, end{input.end()}; it != end;
             ++it, ++count) {
            writer(queue_, current_, *it);

            //  run the kernel
            kernel_(cl::EnqueueArgs(queue_, tetrahedron(dimension_)),
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
    kernel_type kernel_;
    size_t dimension_;

    cl::Buffer current_;
    cl::Buffer previous_;
};
