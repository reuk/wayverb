#pragma once

#include "config.h"
#include "rectangular_mesh.h"
#include "rectangular_program.h"
#include "tetrahedral_mesh.h"
#include "tetrahedral_program.h"

#include "common/callbacks.h"
#include "common/conversions.h"
#include "common/extended_algorithms.h"
#include "common/hrtf.h"
#include "common/progress.h"

#include "glm/glm.hpp"

#include <eigen3/Eigen/LU>
#include <eigen3/Eigen/SVD>

#include <algorithm>
#include <array>
#include <type_traits>

template <typename T>
inline auto pinv(const T& a,
              float epsilon = std::numeric_limits<float>::epsilon()) {
    //  taken from http://eigen.tuxfamily.org/bz/show_bug.cgi?id=257
    Eigen::JacobiSVD<T> svd(a, Eigen::ComputeThinU | Eigen::ComputeThinV);
    auto tolerance = epsilon * std::max(a.cols(), a.rows()) *
                     svd.singularValues().array().abs()(0);
    return svd.matrixV() *
           (svd.singularValues().array().abs() > tolerance)
                   .select(svd.singularValues().array().inverse(), 0)
                   .matrix()
                   .asDiagonal() *
           svd.matrixU().adjoint();
}

//----------------------------------------------------------------------------//

struct RunStepResult {
    explicit RunStepResult(float pressure = 0,
                           const glm::vec3& intensity = glm::vec3())
            : pressure(pressure)
            , intensity(intensity) {
    }
    float pressure;
    glm::vec3 intensity;
};

enum class BufferType { cl, gl };

namespace detail {

template <BufferType buffer_type>
struct BufferTypeTrait;

template <>
struct BufferTypeTrait<BufferType::cl> {
    using type = cl::Buffer;
    using storage_array_type = std::array<type, 2>;

    static storage_array_type create_waveguide_storage(
            const cl::Context& context, size_t nodes);

    static cl::Buffer* index_storage_array(storage_array_type& u, size_t i) {
        return &(u[i]);
    }
};

template <>
struct BufferTypeTrait<BufferType::gl> {
    using type = cl::BufferGL;
    using storage_array_type = std::array<std::pair<type, unsigned int>, 2>;

    static storage_array_type create_waveguide_storage(
            const cl::Context& context, size_t nodes);

    static cl::Buffer* index_storage_array(storage_array_type& u, size_t i) {
        return &(u[i].first);
    }
};

}  // namespace detail

template <typename T, BufferType buffer_type>
class Waveguide {
public:
    using ProgramType = T;
    using kernel_type = decltype(std::declval<ProgramType>().get_kernel());

    using trait = detail::BufferTypeTrait<buffer_type>;
    using storage_array_type = typename trait::storage_array_type;

    Waveguide(const ProgramType& program,
              cl::CommandQueue& queue,
              size_t nodes,
              float sample_rate)
            : queue(queue)
            , kernel(program.get_kernel())
            , nodes(nodes)
            , storage(trait::create_waveguide_storage(
                      program.template get_info<CL_PROGRAM_CONTEXT>(), nodes))
            , previous(trait::index_storage_array(storage, 0))
            , current(trait::index_storage_array(storage, 1))
            , output(program.template get_info<CL_PROGRAM_CONTEXT>(),
                     CL_MEM_READ_WRITE,
                     sizeof(cl_float))
            , sample_rate(sample_rate) {
    }

    virtual ~Waveguide() noexcept = default;

    struct WriteInfo {
        WriteInfo(size_t index, float pressure, bool is_on)
                : index(index)
                , pressure(pressure)
                , is_on(is_on) {
        }
        size_t index;
        float pressure;
        bool is_on;
    };

    virtual RunStepResult run_step(const WriteInfo& write_info,
                                   size_t o,
                                   cl::CommandQueue& queue,
                                   kernel_type& kernel,
                                   size_t nodes,
                                   cl::Buffer& previous,
                                   cl::Buffer& current,
                                   cl::Buffer& output) = 0;

    RunStepResult run_step(size_t o) {
        auto ret = run_step(run_info->get_write_info(),
                            0,
                            queue,
                            kernel,
                            nodes,
                            *previous,
                            *current,
                            output);
        std::swap(current, previous);
        return ret;
    }

    std::pair<RunStepResult, std::vector<cl_float>> run_step_visualised(
            size_t o) {
        auto ret = run_step(o);
        std::vector<cl_float> pressures(nodes, 0);
        cl::copy(queue, *current, pressures.begin(), pressures.end());
        return std::make_pair(ret, pressures);
        ;
    }

    virtual size_t get_index_for_coordinate(const glm::vec3& v) const = 0;
    virtual glm::vec3 get_coordinate_for_index(size_t index) const = 0;

    glm::vec3 get_corrected_coordinate(const glm::vec3& v) const {
        return get_coordinate_for_index(get_index_for_coordinate(v));
    }

    size_t get_nodes() const {
        return nodes;
    }

    virtual bool inside(size_t index) const = 0;

    virtual void setup(cl::CommandQueue& queue, size_t o) {
    }

    void init(const glm::vec3& e, std::vector<float>&& input_sig, size_t o) {
        //  whatever unique setup is required
        setup(queue, o);

        //  zero out meshes
        std::vector<cl_float> n(nodes, 0);
        cl::copy(queue, n.begin(), n.end(), *previous);
        cl::copy(queue, n.begin(), n.end(), *current);

        run_info = std::make_unique<RunInfo>(
                get_index_for_coordinate(e), std::move(input_sig), o);
    }

    template <typename Callback = DoNothingCallback>
    std::vector<RunStepResult> run(size_t steps,
                                   std::atomic_bool& keep_going,
                                   const Callback& callback = Callback()) {
        return run_basic(steps, keep_going, [this, &callback] {
            auto ret = this->run_step(run_info->get_output_index());
            callback();
            return ret;
        });
    }

    template <typename SCallback = DoNothingCallback,
              typename VCallback = GenericArgumentsCallback<std::vector<float>>>
    std::vector<RunStepResult> run_visualised(
            size_t steps,
            std::atomic_bool& keep_going,
            const SCallback& callback = SCallback(),
            const VCallback& visual_callback = VCallback()) {
        return run_basic(
                steps, keep_going, [this, &callback, &visual_callback] {
                    auto ret = this->run_step_visualised(
                            run_info->get_output_index());
                    callback();
                    visual_callback(std::move(ret.second));
                    return ret.first;
                });
    }

    template <typename Callback = DoNothingCallback>
    std::vector<RunStepResult> init_and_run(
            const glm::vec3& e,
            std::vector<float>&& input,
            size_t o,
            size_t steps,
            std::atomic_bool& keep_going,
            const Callback& callback = Callback()) {
        init(e, std::move(input), o);
        return run(steps, keep_going, callback);
    }

    template <typename SCallback = DoNothingCallback,
              typename VCallback = GenericArgumentsCallback<std::vector<float>>>
    std::vector<RunStepResult> init_and_run_visualised(
            const glm::vec3& e,
            std::vector<float>&& input,
            size_t o,
            size_t steps,
            std::atomic_bool& keep_going,
            const SCallback& callback = SCallback(),
            const VCallback& visual_callback = VCallback()) {
        init(e, std::move(input), o);
        return this->run_visualised(
                steps, keep_going, callback, visual_callback);
    }

    cl::CommandQueue& get_queue() const {
        return queue;
    }

    float get_sample_rate() const {
        return sample_rate;
    }

    float get_period() const {
        return 1 / sample_rate;
    }

    std::array<unsigned int, 2> get_gl_indices() const {
        std::array<unsigned int, 2> ret;
        proc::transform(this->storage, ret.begin(), [](const auto& i) {
            return i.second;
        });
        return ret;
    }

private:
    template <typename Callback>
    std::vector<RunStepResult> run_basic(size_t steps,
                                         std::atomic_bool& keep_going,
                                         const Callback& callback) {
        if (!run_info) {
            throw std::runtime_error(
                    "must call init before running waveguide!");
        }

        std::vector<RunStepResult> ret(steps);
        for (auto& i : ret) {
            if (!keep_going) {
                throw std::runtime_error("flag state false, stopping");
            }
            i = callback();
        }

        return ret;
    }

    struct RunInfo final {
        RunInfo(size_t input_index,
                std::vector<float>&& input_signal,
                size_t output_index)
                : input_index(input_index)
                , input_signal(input_signal)
                , output_index(output_index) {
        }

        size_t get_input_index() const {
            return input_index;
        }
        const std::vector<float>& get_input_signal() const {
            return input_signal;
        }
        size_t get_output_index() const {
            return output_index;
        }

        size_t increment_counter() {
            return counter++;
        }

        WriteInfo get_write_info() {
            auto counter = increment_counter();
            auto is_on = counter < get_input_signal().size();
            return WriteInfo(get_input_index(),
                             is_on ? get_input_signal()[counter] : 0,
                             is_on);
        }

    private:
        size_t input_index;
        std::vector<float> input_signal;
        size_t output_index;
        size_t counter{0};
    };

    cl::CommandQueue& queue;
    kernel_type kernel;
    const size_t nodes;

    storage_array_type storage;

    cl::Buffer* previous;
    cl::Buffer* current;

    cl::Buffer output;

    std::unique_ptr<RunInfo> run_info;

    float sample_rate;
};

template <BufferType buffer_type>
class RectangularWaveguide : public Waveguide<RectangularProgram, buffer_type> {
public:
    using Base = Waveguide<RectangularProgram, buffer_type>;

    RectangularWaveguide(const RectangularProgram& program,
                         cl::CommandQueue& queue,
                         const MeshBoundary& boundary,
                         const glm::vec3& anchor,
                         float sr);

    void setup(cl::CommandQueue& queue, size_t o) override;

    RunStepResult run_step(const typename Base::WriteInfo& write_info,
                           size_t o,
                           cl::CommandQueue& queue,
                           typename Base::kernel_type& kernel,
                           size_t nodes,
                           cl::Buffer& previous,
                           cl::Buffer& current,
                           cl::Buffer& output) override;

    size_t get_index_for_coordinate(const glm::vec3& v) const override;
    glm::vec3 get_coordinate_for_index(size_t index) const override;

    const RectangularMesh& get_mesh() const;
    bool inside(size_t index) const override;

private:
    using MeshType = RectangularMesh;
    static constexpr auto PORTS = MeshType::PORTS;

    template <typename T>
    static inline auto get_transform_matrix(
            int o, const T& nodes) {
        Eigen::MatrixXf ret;
        auto count = 0u;
        auto basis = to_vec3f(nodes[o].position);
        for (const auto& i : nodes[o].ports) {
            if (i != RectangularProgram::NO_NEIGHBOR) {
                auto pos = glm::normalize(to_vec3f(nodes[i].position) - basis);
                ret.row(count++) << pos.x, pos.y, pos.z;
            }
        }
        return pinv(ret);
    }

    RectangularWaveguide(const typename Base::ProgramType& program,
                         cl::CommandQueue& queue,
                         const RectangularMesh& mesh,
                         float sample_rate,
                         std::vector<RectangularProgram::CanonicalCoefficients>
                                 coefficients);
    RectangularWaveguide(
            const typename Base::ProgramType& program,
            cl::CommandQueue& queue,
            const RectangularMesh& mesh,
            float sample_rate,
            std::vector<RectangularMesh::CondensedNode> nodes,
            std::vector<RectangularProgram::BoundaryDataArray1> boundary_data_1,
            std::vector<RectangularProgram::BoundaryDataArray2> boundary_data_2,
            std::vector<RectangularProgram::BoundaryDataArray3> boundary_data_3,
            std::vector<RectangularProgram::CanonicalCoefficients>
                    coefficients);

    struct InvocationInfo {
        template <typename T>
        InvocationInfo(int o, const T& nodes)
                : transform_matrix(get_transform_matrix(o, nodes))
                , velocity(0, 0, 0) {
        }

        Eigen::Matrix<float, PORTS, 3> transform_matrix;
        glm::vec3 velocity;
    };

    std::unique_ptr<InvocationInfo> invocation;

    MeshType mesh;
    const cl::Buffer node_buffer;  //  const, set in constructor
    size_t num_boundary_1;
    cl::Buffer boundary_data_1_buffer;
    size_t num_boundary_2;
    cl::Buffer boundary_data_2_buffer;
    size_t num_boundary_3;
    cl::Buffer boundary_data_3_buffer;
    const cl::Buffer
            boundary_coefficients_buffer;  //  const, set in constructor
    cl::Buffer surrounding_buffer;
    std::vector<float> surrounding;
    cl::Buffer error_flag_buffer;
};

template <typename Fun, typename T>
bool is_any(const T& t, const Fun& fun = Fun()) {
    return fun(t);
}

template <typename Fun, typename T>
bool is_any(const std::vector<T>& t, const Fun& fun = Fun()) {
    return proc::any_of(t, [&fun](const auto& i) { return is_any(i, fun); });
}

template <typename Fun, int I>
bool is_any(const RectangularProgram::BoundaryDataArray<I>& t,
            const Fun& fun = Fun()) {
    return proc::any_of(t.array,
                        [&fun](const auto& i) { return is_any(i, fun); });
}

template <typename Fun>
bool is_any(const RectangularProgram::BoundaryData& t, const Fun& fun = Fun()) {
    return proc::any_of(t.filter_memory.array,
                        [&fun](const auto& i) { return is_any(i, fun); });
}

template <typename Fun, typename T>
auto find_any(const T& t, const Fun& fun = Fun()) {
    return proc::find_if(t, [&fun](const auto& i) { return is_any(i, fun); });
}

template <typename Fun, typename T>
auto log_find_any(const T& t,
                  const std::string& identifier,
                  const std::string& func,
                  const Fun& fun = Fun()) {
    auto it = find_any(t, fun);
    if (it != std::end(t)) {
        std::cerr << identifier << " " << func
                  << " index: " << it - std::begin(t) << ", value: " << *it
                  << std::endl;
    }
    return it;
}

template <typename T>
auto log_nan(const T& t, const std::string& identifier) {
    return log_find_any(
            t, identifier, "nan", [](auto i) { return std::isnan(i); });
}

template <typename T>
auto log_inf(const T& t, const std::string& identifier) {
    return log_find_any(
            t, identifier, "inf", [](auto i) { return std::isinf(i); });
}

template <typename T>
auto log_nonzero(const T& t, const std::string& identifier) {
    return log_find_any(t, identifier, "nonzero", [](auto i) { return i; });
}

template <typename T>
bool log_nan_or_nonzero_or_inf(const T& t, const std::string& identifier) {
    if (log_nan(t, identifier) != std::end(t)) {
        return true;
    }
    if (log_inf(t, identifier) != std::end(t)) {
        return true;
    }
    log_nonzero(t, identifier);
    return false;
}
