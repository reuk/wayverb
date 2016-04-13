#pragma once

#include "callbacks.h"
#include "conversions.h"
#include "extended_algorithms.h"
#include "hrtf.h"
#include "power_function.h"
#include "progress.h"
#include "rectangular_mesh.h"
#include "rectangular_program.h"
#include "tetrahedral_mesh.h"
#include "tetrahedral_program.h"

#include <eigen3/Eigen/LU>
#include <eigen3/Eigen/SVD>

#include <glog/logging.h>

#include <algorithm>
#include <array>
#include <type_traits>

template <typename T>
inline T pinv(const T& a,
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

template <typename T>
inline Eigen::MatrixXf get_transform_matrix(int ports, int o, const T& nodes) {
    Eigen::MatrixXf ret(ports, 3);
    auto count = 0u;
    auto basis = to_vec3f(nodes[o].position);
    for (const auto& i : nodes[o].ports) {
        if (i != RectangularProgram::NO_NEIGHBOR) {
            auto pos = (to_vec3f(nodes[i].position) - basis).normalized();
            ret.row(count++) << pos.x, pos.y, pos.z;
        }
    }
    return pinv(ret);
}

//----------------------------------------------------------------------------//

struct RunStepResult {
    RunStepResult(float pressure = 0, const Vec3f& intensity = Vec3f())
            : pressure(pressure)
            , intensity(intensity) {
    }
    float pressure;
    Vec3f intensity;
};

template <typename T>
class Waveguide {
public:
    using ProgramType = T;
    using size_type = std::vector<cl_float>::size_type;
    using kernel_type = decltype(std::declval<ProgramType>().get_kernel());

    Waveguide(const ProgramType& program,
              cl::CommandQueue& queue,
              size_type nodes)
            : queue(queue)
            , kernel(program.get_kernel())
            , nodes(nodes)
            , storage(
                  {{cl::Buffer(program.template getInfo<CL_PROGRAM_CONTEXT>(),
                               CL_MEM_READ_WRITE,
                               sizeof(cl_float) * nodes),
                    cl::Buffer(program.template getInfo<CL_PROGRAM_CONTEXT>(),
                               CL_MEM_READ_WRITE,
                               sizeof(cl_float) * nodes)}})
            , previous(&storage[0])
            , current(&storage[1])
            , output(program.template getInfo<CL_PROGRAM_CONTEXT>(),
                     CL_MEM_READ_WRITE,
                     sizeof(cl_float)) {
    }

    virtual ~Waveguide() noexcept = default;

    virtual RunStepResult run_step(size_type o,
                                   cl::CommandQueue& queue,
                                   kernel_type& kernel,
                                   size_type nodes,
                                   cl::Buffer& previous,
                                   cl::Buffer& current,
                                   cl::Buffer& output) = 0;

    std::vector<cl_float> run_step_slow() {
        run_step(0, queue, kernel, nodes, *previous, *current, output);
        std::vector<cl_float> ret(nodes, 0);
        cl::copy(queue, *previous, ret.begin(), ret.end());
        swap_buffers();
        return ret;
    }

    void swap_buffers() {
        std::swap(current, previous);
    }

    virtual size_type get_index_for_coordinate(const Vec3f& v) const = 0;
    virtual Vec3f get_coordinate_for_index(size_type index) const = 0;

    Vec3f get_corrected_coordinate(const Vec3f& v) const {
        return get_coordinate_for_index(get_index_for_coordinate(v));
    }

    size_type get_nodes() const {
        return nodes;
    }

    virtual bool inside(size_type index) const = 0;

    void initialise_mesh(const PowerFunction& u,
                         const Vec3f& excitation,
                         std::vector<cl_float>& ret) {
        ret.resize(nodes);
        for (auto i = 0u; i != nodes; ++i) {
            if (inside(i))
                ret[i] = u(this->get_coordinate_for_index(i), excitation);
        }
    }

    virtual void setup(cl::CommandQueue& queue, size_type o, float sr) {
    }

    void init(const Vec3f& e, const PowerFunction& u, size_type o, float sr) {
        setup(queue, o, sr);

        std::vector<cl_float> n(nodes, 0);
        cl::copy(queue, n.begin(), n.end(), *previous);

        initialise_mesh(u, e, n);
        bool valid = proc::any_of(n, [](auto i) { return i; });
        if (!valid) {
            throw std::runtime_error("mesh is completely zeroed!");
        }
        cl::copy(queue, n.begin(), n.end(), *current);
    }

    template <typename Callback = DoNothingCallback>
    std::vector<RunStepResult> run(const Vec3f& e,
                                   const PowerFunction& u,
                                   size_type o,
                                   size_type steps,
                                   float sr,
                                   const Callback& callback = Callback()) {
        init(e, u, o, sr);

        std::vector<RunStepResult> ret(steps);
        auto counter = 0u;
        proc::generate(ret, [this, &counter, &steps, &o, &callback] {
            auto ret = this->run_step(
                o, queue, kernel, nodes, *previous, *current, output);

            this->swap_buffers();

            callback();

            return ret;
        });

        return ret;
    }

    template <typename Callback = DoNothingCallback>
    std::vector<RunStepResult> run_basic(
        const Vec3f& e,
        size_type o,
        size_type steps,
        float sr,
        const Callback& callback = Callback()) {
        auto estimated_source_index = get_index_for_coordinate(e);
        auto source_position = get_coordinate_for_index(estimated_source_index);
        return run(
            source_position, BasicPowerFunction(), o, steps, sr, callback);
    }

    template <typename Callback = DoNothingCallback>
    std::vector<RunStepResult> run_gaussian(
        const Vec3f& e,
        size_type o,
        size_type steps,
        float sr,
        const Callback& callback = Callback()) {
        return run(e, GaussianFunction(), o, steps, sr, callback);
    }

    cl::CommandQueue& get_queue() const {
        return queue;
    }

private:
    cl::CommandQueue& queue;
    kernel_type kernel;
    const size_type nodes;

    std::array<cl::Buffer, 2> storage;

    cl::Buffer* previous;
    cl::Buffer* current;

    cl::Buffer output;
};

class RectangularWaveguide : public Waveguide<RectangularProgram> {
public:
#if 0
    template <typename B>
    RectangularWaveguide(const RectangularProgram& program,
                         cl::CommandQueue& queue,
                         const B& boundary,
                         float spacing,
                         const Vec3f& anchor,
                         float sr)
            : RectangularWaveguide(
                  program,
                  queue,
                  RectangularMesh(boundary, spacing, anchor),
                  to_filter_coefficients(boundary.get_surfaces(), sr)) {
    }
#else
    template <typename B>
    RectangularWaveguide(const RectangularProgram& program,
                         cl::CommandQueue& queue,
                         const B& boundary,
                         float spacing,
                         const Vec3f& anchor,
                         float sr)
            : RectangularWaveguide(
                  program,
                  queue,
                  RectangularMesh(boundary, spacing, anchor),
                  std::vector<RectangularProgram::CanonicalCoefficients>{
                      RectangularProgram::CanonicalCoefficients{
                          {1, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0}}}) {
    }
#endif

    void setup(cl::CommandQueue& queue, size_type o, float sr) override;

    RunStepResult run_step(size_type o,
                           cl::CommandQueue& queue,
                           kernel_type& kernel,
                           size_type nodes,
                           cl::Buffer& previous,
                           cl::Buffer& current,
                           cl::Buffer& output) override;

    size_type get_index_for_coordinate(const Vec3f& v) const override;
    Vec3f get_coordinate_for_index(size_type index) const override;

    const RectangularMesh& get_mesh() const;
    bool inside(size_type index) const override;

    template <size_t I>
    static RectangularProgram::FilterDescriptor compute_filter_descriptor(
        const Surface& surface) {
        auto gain =
            decibels::a2db((surface.specular.s[I] + surface.diffuse.s[I]) / 2);
        auto centre = (HrtfData::EDGES[I + 0] + HrtfData::EDGES[I + 1]) / 2;
        //  produce a filter descriptor struct for this filter
        return RectangularProgram::FilterDescriptor{gain, centre, 1.414};
    }

    template <size_t... Ix>
    constexpr static std::array<
        RectangularProgram::FilterDescriptor,
        RectangularProgram::BiquadCoefficientsArray::BIQUAD_SECTIONS>
    to_filter_descriptors(std::index_sequence<Ix...>, const Surface& surface) {
        return {{compute_filter_descriptor<Ix>(surface)...}};
    }

    constexpr static std::array<
        RectangularProgram::FilterDescriptor,
        RectangularProgram::BiquadCoefficientsArray::BIQUAD_SECTIONS>
    to_filter_descriptors(const Surface& surface) {
        return to_filter_descriptors(
            std::make_index_sequence<
                RectangularProgram::BiquadCoefficientsArray::BIQUAD_SECTIONS>(),
            surface);
    }

    static RectangularProgram::CanonicalCoefficients to_filter_coefficients(
        const Surface& surface, float sr) {
        auto descriptors = to_filter_descriptors(surface);
        //  transform filter parameters into a set of biquad coefficients
        auto individual_coeffs =
            RectangularProgram::get_peak_biquads_array(descriptors, sr);
        //  combine biquad coefficients into coefficients for a single
        //  high-order
        //  filter
        auto ret = RectangularProgram::convolve(individual_coeffs);

        //  transform from reflection filter to impedance filter
        return RectangularProgram::to_impedance_coefficients(ret);
    }

    static std::vector<RectangularProgram::CanonicalCoefficients>
    to_filter_coefficients(std::vector<Surface> surfaces, float sr);

private:
    using MeshType = RectangularMesh;
    static constexpr auto PORTS = MeshType::PORTS;
    static constexpr auto TRANSFORM_MATRIX_ELEMENTS = PORTS * 3;

    RectangularWaveguide(
        const ProgramType& program,
        cl::CommandQueue& queue,
        const RectangularMesh& mesh,
        std::vector<RectangularProgram::CanonicalCoefficients> coefficients);
    RectangularWaveguide(
        const ProgramType& program,
        cl::CommandQueue& queue,
        const RectangularMesh& mesh,
        std::vector<RectangularMesh::CondensedNode> nodes,
        std::vector<RectangularProgram::BoundaryDataArray1> boundary_data_1,
        std::vector<RectangularProgram::BoundaryDataArray2> boundary_data_2,
        std::vector<RectangularProgram::BoundaryDataArray3> boundary_data_3,
        std::vector<RectangularProgram::CanonicalCoefficients> coefficients);

    template <int I>
    void setup_boundary_data_buffer(cl::CommandQueue& queue, cl::Buffer& b) {
        std::vector<RectangularProgram::BoundaryDataArray<I>> bda(
            mesh.compute_num_boundary<I>());
        proc::generate(bda, [] {
            RectangularProgram::BoundaryDataArray<I> ret{};
            for (auto& i : ret.array) {
                //  TODO set this properly
                i.coefficient_index = 0;
            }
            return ret;
        });
        cl::copy(queue, bda.begin(), bda.end(), b);
    }

    MeshType mesh;
    const cl::Buffer node_buffer;  //  const, set in constructor
    cl::Buffer transform_buffer;   //  set in setup
    cl::Buffer velocity_buffer;    //  set in setup

    size_t num_boundary_1;
    cl::Buffer boundary_data_1_buffer;  //  set in setup
    size_t num_boundary_2;
    cl::Buffer boundary_data_2_buffer;  //  set in setup
    size_t num_boundary_3;
    cl::Buffer boundary_data_3_buffer;  //  set in setup
    const cl::Buffer
        boundary_coefficients_buffer;  //  const, set in constructor
    cl::Buffer error_flag_buffer;      //  set each iteration

    cl::Buffer debug_buffer;

    float period;
};

class TetrahedralWaveguide : public Waveguide<TetrahedralProgram> {
public:
    TetrahedralWaveguide(const ProgramType& program,
                         cl::CommandQueue& queue,
                         const Boundary& boundary,
                         float spacing,
                         const Vec3f& anchor);

    void setup(cl::CommandQueue& queue, size_type o, float sr) override;

    RunStepResult run_step(size_type o,
                           cl::CommandQueue& queue,
                           kernel_type& kernel,
                           size_type nodes,
                           cl::Buffer& previous,
                           cl::Buffer& current,
                           cl::Buffer& output) override;

    size_type get_index_for_coordinate(const Vec3f& v) const override;
    Vec3f get_coordinate_for_index(size_type index) const override;

    const TetrahedralMesh& get_mesh() const;
    bool inside(size_type index) const override;

private:
    using MeshType = TetrahedralMesh;
    static constexpr auto PORTS = MeshType::PORTS;
    static constexpr auto TRANSFORM_MATRIX_ELEMENTS = PORTS * 3;

    TetrahedralWaveguide(const ProgramType& program,
                         cl::CommandQueue& queue,
                         const TetrahedralMesh& mesh);
    TetrahedralWaveguide(const ProgramType& program,
                         cl::CommandQueue& queue,
                         const TetrahedralMesh& mesh,
                         std::vector<TetrahedralMesh::Node> nodes);

    MeshType mesh;
    cl::Buffer node_buffer;
    cl::Buffer transform_buffer;
    cl::Buffer velocity_buffer;

    float period;
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
        LOG(INFO) << identifier << " " << func
                  << " index: " << it - std::begin(t) << ", value: " << *it;
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
    if (log_nan(t, identifier) != std::end(t))
        return true;
    if (log_inf(t, identifier) != std::end(t))
        return true;
    log_nonzero(t, identifier);
    return false;
}
