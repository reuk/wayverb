#pragma once

#include "waveguide/waveguide.h"

struct rectangular_waveguide_run_info;

class RectangularWaveguide : public Waveguide<RectangularProgram> {
public:
    using Base = Waveguide<RectangularProgram>;

    RectangularWaveguide(const RectangularProgram& program,
                         const MeshBoundary& boundary,
                         const glm::vec3& anchor,
                         float sr);
    virtual ~RectangularWaveguide() noexcept;
    const RectangularMesh& get_mesh() const;

    size_t get_index_for_coordinate(const glm::vec3& v) const override;
    glm::vec3 get_coordinate_for_index(size_t index) const override;
    bool inside(size_t index) const override;

private:
    using MeshType              = RectangularMesh;
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
    RectangularWaveguide(const typename Base::ProgramType& program,
                         const RectangularMesh& mesh,
                         float sample_rate,
                         std::vector<RectangularMesh::CondensedNode>
                                 nodes,
                         std::vector<RectangularProgram::CanonicalCoefficients>
                                 coefficients);

    std::unique_ptr<rectangular_waveguide_run_info> invocation;

    const MeshType mesh;
    const cl::Buffer node_buffer;                   //  const
    const cl::Buffer boundary_coefficients_buffer;  //  const
    std::vector<cl_float>
            surrounding;            //  overwritten every step, constant size
    cl::Buffer surrounding_buffer;  //  overwritten every step, constant size
    cl::Buffer error_flag_buffer;   //  overwritten every step, constant size

    friend bool operator==(const RectangularWaveguide& a,
                           const RectangularWaveguide& b);
};

bool operator==(const RectangularWaveguide& a, const RectangularWaveguide& b);
