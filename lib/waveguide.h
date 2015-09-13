#pragma once

#include "rectangular_program.h"
#include "recursive_tetrahedral_program.h"
#include "iterative_tetrahedral_program.h"
#include "recursive_tetrahedral.h"

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#include <array>
#include <type_traits>

#define TESTING

class RectangularWaveguide {
public:
    using size_type = std::vector<cl_float>::size_type;
    using kernel_type =
        decltype(std::declval<RectangularProgram>().get_kernel());

    RectangularWaveguide(const RectangularProgram & program,
                         cl::CommandQueue & queue,
                         cl_int3 p);

    std::vector<cl_float> run(std::vector<float> input,
                              cl_int3 excitation,
                              cl_int3 read_head,
                              cl_float attenuation,
                              int steps);

private:
    cl::CommandQueue & queue;

    kernel_type kernel;

    size_type get_index(cl_int3 pos) const;

    const cl_int3 p;

    std::array<cl::Buffer, 3> storage;

    cl::Buffer & previous;
    cl::Buffer & current;
    cl::Buffer & next;

    cl::Buffer output;
};

class RecursiveTetrahedralWaveguide {
public:
    using size_type = std::vector<cl_float>::size_type;
    using kernel_type =
        decltype(std::declval<RecursiveTetrahedralProgram>().get_kernel());

    RecursiveTetrahedralWaveguide(const RecursiveTetrahedralProgram & program,
                                  cl::CommandQueue & queue,
                                  const Boundary & boundary,
                                  Vec3f start,
                                  float spacing);

    std::vector<cl_float> run(std::vector<float> input,
                              size_type excitation,
                              size_type read_head,
                              cl_float attenuation,
                              int steps);

private:
    RecursiveTetrahedralWaveguide(const RecursiveTetrahedralProgram & program,
                                  cl::CommandQueue & queue,
                                  std::vector<LinkedTetrahedralNode> nodes);

    cl::CommandQueue & queue;

    kernel_type kernel;

#ifdef TESTING
    std::vector<LinkedTetrahedralNode> & nodes;
#endif

    const size_type node_size;
    cl::Buffer node_buffer;

    std::array<cl::Buffer, 3> storage;

    cl::Buffer & previous;
    cl::Buffer & current;
    cl::Buffer & next;

    cl::Buffer output;
};

class IterativeTetrahedralWaveguide {
public:
    using size_type = std::vector<cl_float>::size_type;
    using kernel_type =
        decltype(std::declval<IterativeTetrahedralProgram>().get_kernel());

    class Locator {
    public:
        Locator(const Vec3i & pos = Vec3i(), int mod_ind = 0)
                : pos(pos), mod_ind(mod_ind) {}
        Vec3i pos;
        int mod_ind;
    };

    IterativeTetrahedralWaveguide(const IterativeTetrahedralProgram & program,
                                  cl::CommandQueue & queue,
                                  const Boundary & boundary,
                                  float cube_side);

    std::vector<cl_float> run(std::vector<float> input,
                              size_type excitation,
                              size_type read_head,
                              cl_float attenuation,
                              int steps);
private:
    static std::vector<Vec3f> get_scaled_cube(float scale);

    size_type get_index(const Locator & locator);
    Locator get_locator(size_type index);

    std::vector<size_type> get_absolute_neighbors(size_type index);

    cl::CommandQueue & queue;

    kernel_type kernel;

    Vec3i dim;

#ifdef TESTING
    std::vector<UnlinkedTetrahedralNode> & nodes;
#endif

    const size_type node_size;
    cl::Buffer node_buffer;

    std::array<cl::Buffer, 3> storage;

    cl::Buffer & previous;
    cl::Buffer & current;
    cl::Buffer & next;

    cl::Buffer output;

    static const std::vector<Vec3f> basic_cube;
    static const std::vector<std::vector<Locator>> offset_table;

    const std::vector<Vec3f> scaled_cube;
};
