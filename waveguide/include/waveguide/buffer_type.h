#pragma once

#include "cl.hpp"

#include <array>

enum class BufferType { cl, gl };

//----------------------------------------------------------------------------//

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
