#pragma once

#include "waveguide/waveguide.h"

template <BufferType buffer_type>
class RectangularWaveguide : public Waveguide<RectangularProgram, buffer_type> {
public:
    using Base = Waveguide<RectangularProgram, buffer_type>;

    RectangularWaveguide(const RectangularProgram& program,
                         const MeshBoundary& boundary,
                         const glm::vec3& anchor,
                         float sr);
    const RectangularMesh& get_mesh() const;

    size_t get_index_for_coordinate(const glm::vec3& v) const override;
    glm::vec3 get_coordinate_for_index(size_t index) const override;
    bool inside(size_t index) const override;

private:
    using MeshType = RectangularMesh;
    static constexpr auto PORTS = MeshType::PORTS;

    RunStepResult run_step(const typename Base::WriteInfo& write_info,
                           size_t o,
                           cl::CommandQueue& queue,
                           typename Base::kernel_type& kernel,
                           size_t nodes,
                           cl::Buffer& previous,
                           cl::Buffer& current,
                           cl::Buffer& output) override;
    void setup(cl::CommandQueue& queue, size_t o) override;

    RectangularWaveguide(const typename Base::ProgramType& program,
                         const RectangularMesh& mesh,
                         float sample_rate,
                         std::vector<RectangularProgram::CanonicalCoefficients>
                                 coefficients);
    RectangularWaveguide(
            const typename Base::ProgramType& program,
            const RectangularMesh& mesh,
            float sample_rate,
            std::vector<RectangularMesh::CondensedNode> nodes,
            std::vector<RectangularProgram::CanonicalCoefficients>
                    coefficients);

    struct invocation_info {
        struct buffer_size_pair {
            template <typename T>
            buffer_size_pair(const cl::Context& context, std::vector<T> u)
                    : size(u.size())
                    , buffer(context, u.begin(), u.end(), false) {
            }

            const size_t size;
            cl::Buffer buffer;
        };

        template <typename T>
        invocation_info(
                int o,
                const T& nodes,
                const cl::Context& context,
                const std::vector<RectangularProgram::BoundaryDataArray1>& bd1,
                const std::vector<RectangularProgram::BoundaryDataArray2>& bd2,
                const std::vector<RectangularProgram::BoundaryDataArray3>& bd3)
                : transform_matrix(
                          detail::get_transform_matrix(PORTS, o, nodes))
                , velocity(0, 0, 0)
                , boundary_1(context, bd1)
                , boundary_2(context, bd2)
                , boundary_3(context, bd3)
        {
        }

        using original_matrix_type = Eigen::MatrixXf;
        using transform_matrix_type = Eigen::MatrixXf;
        transform_matrix_type transform_matrix;
        glm::vec3 velocity;
        buffer_size_pair boundary_1;
        buffer_size_pair boundary_2;
        buffer_size_pair boundary_3;
    };

    std::unique_ptr<invocation_info> invocation;

    const MeshType mesh;
    const cl::Buffer node_buffer;                   //  const
    const cl::Buffer boundary_coefficients_buffer;  //  const
    cl::Buffer surrounding_buffer;   //  overwritten every step, constant size
    std::vector<float> surrounding;  //  overwritten every step, constant size
    cl::Buffer error_flag_buffer;    //  overwritten every step, constant size
};
