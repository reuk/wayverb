#pragma once

#include "config.h"
#include "rectangular_mesh.h"
#include "rectangular_program.h"
#include "tetrahedral_mesh.h"
#include "tetrahedral_program.h"

#include "common/conversions.h"
#include "common/extended_algorithms.h"
#include "common/hrtf.h"
#include "common/progress.h"

#include "glm/glm.hpp"

#include <algorithm>
#include <array>
#include <type_traits>

struct RunStepResult {
    explicit RunStepResult(float pressure             = 0,
                           const glm::vec3& intensity = glm::vec3())
            : pressure(pressure)
            , intensity(intensity) {
    }
    float pressure;
    glm::vec3 intensity;
};

template <typename T>
class Waveguide {
public:
    using ProgramType = T;
    using kernel_type = decltype(std::declval<ProgramType>().get_kernel());

    Waveguide(const ProgramType& program, size_t nodes, double sample_rate)
            : program(program)
            , queue(program.template get_info<CL_PROGRAM_CONTEXT>(),
                    program.get_device())
            , kernel(program.get_kernel())
            , nodes(nodes)
            , previous(program.template get_info<CL_PROGRAM_CONTEXT>(),
                       CL_MEM_READ_WRITE,
                       nodes * sizeof(cl_float))
            , current(program.template get_info<CL_PROGRAM_CONTEXT>(),
                      CL_MEM_READ_WRITE,
                      nodes * sizeof(cl_float))
            , output(program.template get_info<CL_PROGRAM_CONTEXT>(),
                     CL_MEM_READ_WRITE,
                     sizeof(cl_float))
            , sample_rate(sample_rate) {
    }

    virtual ~Waveguide() noexcept = default;

    glm::vec3 get_corrected_coordinate(const glm::vec3& v) const {
        return get_coordinate_for_index(get_index_for_coordinate(v));
    }

    size_t get_nodes() const {
        return nodes;
    }

    using PerStepCallback    = std::function<void()>;
    using VisualiserCallback = std::function<void(std::vector<float>)>;

    std::vector<RunStepResult> init_and_run(const glm::vec3& e,
                                            const std::vector<float>& input,
                                            size_t o,
                                            size_t steps,
                                            std::atomic_bool& keep_going,
                                            const PerStepCallback& callback) {
        auto run_info = init(e, input, o, steps);
        return this->run(run_info, keep_going, callback);
    }

    std::vector<RunStepResult> init_and_run_visualised(
            const glm::vec3& e,
            const std::vector<float>& input,
            size_t o,
            size_t steps,
            std::atomic_bool& keep_going,
            const PerStepCallback& callback,
            const VisualiserCallback& visual_callback) {
        auto run_info = init(e, input, o, steps);
        return this->run_visualised(
                run_info, keep_going, callback, visual_callback);
    }

    /*
    cl::CommandQueue& get_queue() const {
        return queue;
    }
    */

    double get_sample_rate() const {
        return sample_rate;
    }

    double get_period() const {
        return 1.0 / sample_rate;
    }

    std::array<unsigned int, 2> get_gl_indices() const {
        std::array<unsigned int, 2> ret;
        proc::transform(this->storage, ret.begin(), [](const auto& i) {
            return i.second;
        });
        return ret;
    }

    ProgramType get_program() const {
        return program;
    }

    virtual size_t get_index_for_coordinate(const glm::vec3& v) const = 0;
    virtual glm::vec3 get_coordinate_for_index(size_t index) const    = 0;
    virtual bool inside(size_t index) const                           = 0;

protected:
    struct WriteInfo {
        WriteInfo(size_t index, float pressure)
                : index(index)
                , pressure(pressure) {
        }
        const size_t index;
        const float pressure;
    };

private:
    virtual RunStepResult run_step(const WriteInfo& write_info,
                                   size_t o,
                                   cl::CommandQueue& queue,
                                   kernel_type& kernel,
                                   size_t nodes,
                                   cl::Buffer& previous,
                                   cl::Buffer& current,
                                   cl::Buffer& output) = 0;
    virtual void setup(cl::CommandQueue& queue, size_t o) = 0;

    template <typename InIt, typename OutIt, typename Fun>
    static void ordered_transform(InIt a, InIt b, OutIt x, Fun&& t) {
        for (; a != b; ++a) {
            *x = t(*a);
        }
    }

    struct RunInfo final {
        RunInfo(size_t input_index,
                const std::vector<float>& sig,
                size_t output_index)
                : input_index(input_index)
                , input_signal(sig)
                , output_index(output_index) {
        }

        const size_t input_index;
        const std::vector<float> input_signal;
        const size_t output_index;
    };

    using InputCallback = std::function<RunStepResult(float)>;

    std::vector<RunStepResult> run_basic(
            const RunInfo& run_info,
            std::atomic_bool& keep_going,
            const InputCallback& callback = InputCallback()) {
        std::vector<RunStepResult> ret;
        ret.reserve(run_info.input_signal.size());

        //  I would use std::transform here but I need to guarantee order
        ordered_transform(
                run_info.input_signal.begin(),
                run_info.input_signal.end(),
                std::back_inserter(ret),
                [&callback, &keep_going](auto i) {
                    if (!keep_going) {
                        throw std::runtime_error("flag state false, stopping");
                    }
                    return callback(i);
                });

        return ret;
    }

    RunInfo init(const glm::vec3& e,
                 const std::vector<float>& input_sig,
                 size_t o,
                 size_t steps) {
        //  whatever unique setup is required
        setup(queue, o);

        auto zero_mesh = [this](auto& buffer) {
            std::vector<cl_uchar> n(buffer.template getInfo<CL_MEM_SIZE>(), 0);
            cl::copy(queue, n.begin(), n.end(), buffer);
        };
        zero_mesh(previous);
        zero_mesh(current);

        auto t = input_sig;
        t.resize(steps, 0);
        return RunInfo(get_index_for_coordinate(e), t, o);
    }

    RunStepResult run_step(const RunInfo& run_info, float input) {
        auto ret = run_step(WriteInfo(run_info.input_index, input),
                            run_info.output_index,
                            queue,
                            kernel,
                            nodes,
                            previous,
                            current,
                            output);
        std::swap(current, previous);
        return ret;
    }

    std::pair<RunStepResult, std::vector<cl_float>> run_step_visualised(
            const RunInfo& run_info, float input) {
        auto ret = run_step(run_info, input);
        std::vector<cl_float> pressures(nodes, 0);
        cl::copy(queue, previous, pressures.begin(), pressures.end());
        return std::make_pair(ret, pressures);
        ;
    }

    std::vector<RunStepResult> run(const RunInfo& ri,
                                   std::atomic_bool& keep_going,
                                   const PerStepCallback& callback) {
        return run_basic(ri, keep_going, [this, &ri, &callback](auto i) {
            auto ret = this->run_step(ri, i);
            callback();
            return ret;
        });
    }

    std::vector<RunStepResult> run_visualised(
            const RunInfo& ri,
            std::atomic_bool& keep_going,
            const PerStepCallback& callback,
            const VisualiserCallback& visual_callback) {
        return run_basic(ri,
                         keep_going,
                         [this, &ri, &callback, &visual_callback](auto i) {
                             auto ret = this->run_step_visualised(ri, i);
                             callback();
                             visual_callback(ret.second);
                             return ret.first;
                         });
    }

    ProgramType program;
    cl::CommandQueue queue;
    kernel_type kernel;
    const size_t nodes;

    cl::Buffer previous;
    cl::Buffer current;

    cl::Buffer output;

    double sample_rate;
};
