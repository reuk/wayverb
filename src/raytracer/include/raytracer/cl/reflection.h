#pragma once

#include "core/cl/traits.h"

#include <tuple>

namespace wayverb {
namespace raytracer {

struct alignas(1 << 4) reflection final {
    cl_float3 position;   //  position of the secondary source
    cl_uint triangle;     //  triangle which contains source
    cl_char keep_going;   //  whether or not this is the teriminator for this
                          //  path (like a \0 in a char*)
    cl_char receiver_visible;  //  whether or not the receiver is visible from
                               //  this point
};

constexpr auto to_tuple(const reflection& x) {
    return std::tie(x.position,
                    x.triangle,
                    x.keep_going,
                    x.receiver_visible);
}

constexpr bool operator==(const reflection& a, const reflection& b) {
    return to_tuple(a) == to_tuple(b);
}

constexpr bool operator!=(const reflection& a, const reflection& b) {
    return !(a == b);
}

}  // namespace raytracer
}  // namespace wayverb
