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
            std::vector<RectangularProgram::BoundaryDataArray1> boundary_data_1,
            std::vector<RectangularProgram::BoundaryDataArray2> boundary_data_2,
            std::vector<RectangularProgram::BoundaryDataArray3> boundary_data_3,
            std::vector<RectangularProgram::CanonicalCoefficients>
                    coefficients);

    struct InvocationInfo {
        template <typename T>
        InvocationInfo(int o, const T& nodes)
                : transform_matrix(
                          detail::get_transform_matrix(PORTS, o, nodes))
                , velocity(0, 0, 0) {
        }

        using original_matrix_type = Eigen::MatrixXf;
        using transform_matrix_type = Eigen::MatrixXf;
        transform_matrix_type transform_matrix;
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
