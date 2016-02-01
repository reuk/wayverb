#pragma once

#include "vec.h"
#include "cl_structs.h"
#include "boundaries.h"
#include "waveguide.h"

#include "rectangular_mesh.h"

#include <vector>

class CpuRectangularWaveguide final {
public:
    using Node = NodeStruct<6>;
    using Collection = std::vector<float>;
    using size_type = Collection::size_type;
    static constexpr int PORTS = Node::PORTS;

    CpuRectangularWaveguide(const Boundary& boundary,
                            float spacing,
                            const Vec3f& anchor);

    const RectangularMesh& get_mesh() const;

    void swap_buffers();

    void initialise_mesh(const PowerFunction& u,
                         const Vec3f& excitation,
                         std::vector<cl_float>& ret);

    void init(const Vec3f& e, const PowerFunction& u, size_type o, float sr);

    void setup(size_type o, float sr);

    std::vector<RunStepResult> run(const Vec3f& e,
                                   const PowerFunction& u,
                                   size_type o,
                                   size_type steps,
                                   float sr);

    RunStepResult run_step(size_type o,
                           Collection& previous,
                           const Collection& current);

    Collection run_step_slow();

private:
    RectangularMesh mesh;

    std::array<Collection, 2> storage;

    Collection* previous;
    Collection* current;

    Eigen::MatrixXf transform_matrix;
    Vec3f velocity_buffer;
    float period;
};
