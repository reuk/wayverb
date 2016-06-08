#include "ComputeIndices.hpp"

#include "common/stl_wrappers.h"

std::vector<GLuint> compute_indices(GLuint num) {
    std::vector<GLuint> ret(num);
    proc::iota(ret, 0);
    return ret;
}