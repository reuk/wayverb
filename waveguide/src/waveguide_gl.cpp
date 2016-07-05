#include "waveguide/rectangular_waveguide.h"

#include <OpenGL/gl3.h>

namespace detail {

BufferTypeTrait<BufferType::gl>::storage_array_type
BufferTypeTrait<BufferType::gl>::create_waveguide_storage(
        const cl::Context& context, size_t nodes) {
    storage_array_type ret;
    for (auto& i : ret) {
        glGenBuffers(1, &i.second);
        glBindBuffer(GL_ARRAY_BUFFER, i.second);
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(cl_float) * nodes,
                     nullptr,
                     GL_DYNAMIC_DRAW);
        cl_int err{0};
        i.first = cl::BufferGL(context, CL_MEM_READ_WRITE, i.second, &err);
    }
    return ret;
}

}  // namespace detail

template class RectangularWaveguide<BufferType::gl>;
