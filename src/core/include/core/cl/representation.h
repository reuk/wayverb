#pragma once

namespace wayverb {
namespace core {

template <typename t>
struct cl_representation;

template <typename t>
const auto cl_representation_v = cl_representation<t>::value;

}  // namespace core
}  // namespace wayverb
