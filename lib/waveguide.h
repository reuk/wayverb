#pragma once

#include "tetrahedral_program.h"
#include "rectangular_program.h"
#include "rectangular_mesh.h"
#include "iterative_tetrahedral_mesh.h"
#include "cl_structs.h"
#include "logger.h"
#include "conversions.h"
#include "power_function.h"

#include <eigen3/Eigen/LU>
#include <eigen3/Eigen/SVD>

#include <array>
#include <type_traits>
#include <algorithm>

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
        if (i >= 0) {
            auto pos = (to_vec3f(nodes[i].position) - basis).normalized();
            ret.row(count++) << pos.x, pos.y, pos.z;
        }
    }
    return pinv(ret);
}

//----------------------------------------------------------------------------//

struct RunStepResult {
    RunStepResult(float pressure = 0, const Vec3f& intensity = 0)
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
        cl::copy(queue, n.begin(), n.end(), *current);
    }

    std::vector<RunStepResult> run(const Vec3f& e,
                                   const PowerFunction& u,
                                   size_type o,
                                   size_type steps,
                                   float sr) {
        init(e, u, o, sr);

        std::vector<RunStepResult> ret(steps);
        auto counter = 0u;
        std::generate(
            ret.begin(),
            ret.end(),
            [this, &counter, &steps, &o] {
                auto ret = this->run_step(
                    o, queue, kernel, nodes, *previous, *current, output);

                this->swap_buffers();

                auto percent = counter * 100 / (steps - 1);
                std::cout << "\r" << percent << "% done" << std::flush;

                counter += 1;

                return ret;
            });

        std::cout << std::endl;

        return ret;
    }

    std::vector<RunStepResult> run_basic(const Vec3f& e,
                                         size_type o,
                                         size_type steps,
                                         float sr) {
        auto estimated_source_index = get_index_for_coordinate(e);
        auto source_position = get_coordinate_for_index(estimated_source_index);
        return run(source_position, BasicPowerFunction(), o, steps, sr);
    }

    std::vector<RunStepResult> run_inverse(
        const Vec3f& e, float power, size_type o, size_type steps, float sr) {
        return run(e, InversePowerFunction(power), o, steps, sr);
    }

    std::vector<RunStepResult> run_inverse_square(
        const Vec3f& e, float power, size_type o, size_type steps, float sr) {
        return run(e, InverseSquarePowerFunction(power), o, steps, sr);
    }

    std::vector<RunStepResult> run_gaussian(const Vec3f& e,
                                            size_type o,
                                            size_type steps,
                                            float sr) {
        return run(e, GaussianFunction(), o, steps, sr);
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
    RectangularWaveguide(const ProgramType& program,
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

    const RectangularMesh& get_mesh() const;
    bool inside(size_type index) const override;

private:
    RectangularWaveguide(const ProgramType& program,
                         cl::CommandQueue& queue,
                         const RectangularMesh& mesh);

    RectangularMesh mesh;
    cl::Buffer node_buffer;
    cl::Buffer transform_buffer;
    cl::Buffer velocity_buffer;

    cl::Buffer boundary_data_1_buffer;
    cl::Buffer boundary_data_2_buffer;
    cl::Buffer boundary_data_3_buffer;

    Eigen::MatrixXf transform_matrix;

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

    const IterativeTetrahedralMesh& get_mesh() const;
    bool inside(size_type index) const override;

private:
    TetrahedralWaveguide(const ProgramType& program,
                         cl::CommandQueue& queue,
                         const IterativeTetrahedralMesh& mesh);

    IterativeTetrahedralMesh mesh;
    cl::Buffer node_buffer;
    cl::Buffer transform_buffer;
    cl::Buffer velocity_buffer;
    Eigen::MatrixXf transform_matrix;

    float period;
};
